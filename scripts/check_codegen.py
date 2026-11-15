#!/usr/bin/env python3
"""Check the machine code a compiler generates for a codegen probe.

A probe is a small source file under ``tests/codegen/`` that defines one
function, ``ptn_codegen_probe``, containing a single match. This script
compiles the probe to assembly with optimization enabled and fails when
the function

- calls or tail-jumps to another function,
- reserves more stack than its limit,
- writes to the stack more often than its limit,
- has more conditional branches than its limit, when one is given, or
- has more instructions than its budget, when one is given.

The first three are the signature of a match that stayed partly out of
line. The compiler can then no longer treat the case pack as constants:
it writes every case to the stack before dispatching, and the cost of
the match grows with the number of cases.

Stack use is the main signal because it does not depend on how the
compiler lowers a ``switch``. A fully inlined dense match may become a
lookup table (a handful of instructions) or a jump table (about two
instructions per case); both are fine and neither touches the stack.
The instruction budget is therefore optional and meant only for probes
whose code size does not grow with the number of cases.

The branch limit catches the other way a keyed match can degrade: a
chain or tree of comparisons instead of one indexed jump. That uses no
stack and makes no call, but it has a conditional branch per step,
while a table has only its bounds check however it is lowered.

Unlike a timing benchmark this check is deterministic, so it can block a
pull request.

Usage:
    check_codegen.py --compiler g++ --include include \\
        --source tests/codegen/literal_dense_64.cpp
"""

from __future__ import annotations

import argparse
import re
import subprocess
import sys

DEFAULT_FUNCTION = "ptn_codegen_probe"

# x86 and AArch64 call instructions.
CALL_RE = re.compile(r"^(call[a-z]*|bl|blr)$")
# Direct jumps and branches. A direct jump whose target is not a local
# label leaves the function, which is how a tail call shows up. Indirect
# jumps (``jmp *%rax``, ``br x0``) are jump tables and stay inside it.
JUMP_RE = re.compile(r"^(j[a-z]+|b|b\.[a-z]+)$")
LOCAL_TARGET_RE = re.compile(r"^(\*|\.?L|\d)")
# Conditional branches: x86 `jcc` (every `j...` except `jmp`) and the
# AArch64 `b.cond`, compare-and-branch and test-and-branch forms.
CONDITIONAL_RE = re.compile(
    r"^(j(?!mp)[a-z]+|b\.[a-z]+|cbn?z|tbn?z)$"
)
LABEL_RE = re.compile(r"^[^\s#;/]\S*:")
# ``#`` starts a comment only when followed by a space or another ``#``;
# on AArch64 it also introduces an immediate (``#144``).
COMMENT_RE = re.compile(r"\s*(#(\s|#)|;|//).*$")

# Stack reservation: ``subq $N, %rsp``, ``addq $-N, %rsp``,
# ``sub sp, sp, #N`` and the pre-indexed ``stp x29, x30, [sp, -N]!``.
X86_SUB_RSP_RE = re.compile(r"^subq?\s+\$(\d+),\s*%rsp$")
X86_ADD_RSP_RE = re.compile(r"^addq?\s+\$-(\d+),\s*%rsp$")
ARM_SUB_SP_RE = re.compile(r"^sub\s+sp,\s*sp,\s*#?(\d+)")
ARM_PREINDEX_RE = re.compile(r"\[sp,\s*#?-(\d+)\]!")

# Stack writes: an x86 instruction whose destination is memory addressed
# through %rsp or %rbp, or an AArch64 store addressed through sp or x29.
# x86 leaf functions may write below %rsp without reserving anything
# (the red zone), so writes are counted separately from the reservation.
X86_STACK_DEST_RE = re.compile(r",\s*-?\w*\(%r[sb]p[^)]*\)$")
X86_NO_WRITE_RE = re.compile(r"^(v?p?cmp|v?p?test|v?u?comis|bt\b)")
ARM_STORE_RE = re.compile(r"^(str|stp|stur)[bh]?\s.*\[(sp|x29)\b")


def compile_to_assembly(args: argparse.Namespace) -> str:
    command = [
        args.compiler,
        f"-std={args.std}",
        f"-{args.opt}",
        "-S",
        "-o",
        "-",
    ]
    for directory in args.include:
        command += ["-I", directory]
    command += args.flag
    command.append(args.source)

    result = subprocess.run(
        command, capture_output=True, text=True, check=False
    )
    if result.returncode != 0:
        sys.stderr.write(result.stderr)
        sys.exit(f"error: failed to compile {args.source}")
    return result.stdout


def function_body(assembly: str, function: str) -> list[str] | None:
    """Return the lines between the function's label and its end."""
    start_re = re.compile(rf"^_?{re.escape(function)}:")
    end_re = re.compile(
        rf"^\s*(\.cfi_endproc\b|\.size\s+_?{re.escape(function)}\b)"
    )

    body: list[str] | None = None
    for line in assembly.splitlines():
        if body is None:
            if start_re.match(line):
                body = []
            continue
        if end_re.match(line):
            return body
        body.append(line)
    return body


def instructions(body: list[str]) -> list[str]:
    """Drop labels, directives, comments and blank lines."""
    found = []
    for line in body:
        text = COMMENT_RE.sub("", line).strip()
        if not text or LABEL_RE.match(line) or text.startswith("."):
            continue
        found.append(text)
    return found


def is_escape(text: str) -> bool:
    """True when the instruction transfers control out of the function."""
    fields = text.split()
    mnemonic = fields[0]
    if CALL_RE.match(mnemonic):
        return True
    if JUMP_RE.match(mnemonic) and len(fields) > 1:
        return not LOCAL_TARGET_RE.match(fields[-1])
    return False


def stack_bytes(found: list[str]) -> int:
    """Largest stack reservation made by a single instruction."""
    largest = 0
    for text in found:
        for pattern in (
            X86_SUB_RSP_RE,
            X86_ADD_RSP_RE,
            ARM_SUB_SP_RE,
            ARM_PREINDEX_RE,
        ):
            match = pattern.search(text)
            if match:
                largest = max(largest, int(match.group(1)))
    return largest


def stack_stores(found: list[str]) -> list[str]:
    """Instructions that write to the stack."""
    stores = []
    for text in found:
        if ARM_STORE_RE.match(text):
            stores.append(text)
        elif X86_STACK_DEST_RE.search(text) and not X86_NO_WRITE_RE.match(
            text
        ):
            stores.append(text)
    return stores


def conditional_branches(found: list[str]) -> list[str]:
    """Instructions that branch on a condition."""
    return [
        text for text in found if CONDITIONAL_RE.match(text.split()[0])
    ]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("--compiler", required=True)
    parser.add_argument("--source", required=True)
    parser.add_argument("--include", action="append", default=[])
    parser.add_argument("--flag", action="append", default=[])
    parser.add_argument("--std", default="c++17")
    parser.add_argument("--opt", default="O2")
    parser.add_argument("--function", default=DEFAULT_FUNCTION)
    parser.add_argument("--max-stack-bytes", type=int, default=64)
    parser.add_argument("--max-stack-stores", type=int, default=8)
    parser.add_argument("--max-branches", type=int, default=None)
    parser.add_argument("--max-instructions", type=int, default=None)
    args = parser.parse_args()

    body = function_body(compile_to_assembly(args), args.function)
    if body is None:
        sys.exit(
            f"error: {args.function} not found in the assembly of "
            f"{args.source}"
        )

    found = instructions(body)
    reserved = stack_bytes(found)
    stores = stack_stores(found)
    escapes = [text for text in found if is_escape(text)]
    branches = conditional_branches(found)
    problems = []

    if escapes:
        problems.append(
            f"{len(escapes)} call(s) to out-of-line code, "
            f"first: {escapes[0][:120]}"
        )
    if reserved > args.max_stack_bytes:
        problems.append(
            f"reserves {reserved} bytes of stack, limit is "
            f"{args.max_stack_bytes}"
        )
    if len(stores) > args.max_stack_stores:
        problems.append(
            f"{len(stores)} writes to the stack, limit is "
            f"{args.max_stack_stores}"
        )
    if (
        args.max_branches is not None
        and len(branches) > args.max_branches
    ):
        problems.append(
            f"{len(branches)} conditional branches, limit is "
            f"{args.max_branches}"
        )
    if (
        args.max_instructions is not None
        and len(found) > args.max_instructions
    ):
        problems.append(
            f"{len(found)} instructions, budget is "
            f"{args.max_instructions}"
        )

    print(
        f"{args.source}: {len(found)} instructions, {reserved} stack "
        f"bytes, {len(stores)} stack writes, {len(branches)} "
        f"branches, {len(escapes)} calls"
    )
    if not problems:
        return 0

    for problem in problems:
        print(f"FAIL: {problem}")
    print(
        "The match is no longer fully inlined or no longer uses "
        "keyed dispatch. See include/ptn/core/common/inline.hpp and "
        "index_switch.hpp."
    )
    print("--- generated code (first 60 instructions) ---")
    for text in found[:60]:
        print(f"    {text[:140]}")
    return 1


if __name__ == "__main__":
    sys.exit(main())
