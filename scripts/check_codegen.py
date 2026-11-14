#!/usr/bin/env python3
"""Check the machine code a compiler generates for a codegen probe.

A probe is a small source file under ``tests/codegen/`` that defines one
function, ``ptn_codegen_probe``, containing a single match. This script
compiles the probe to assembly with optimization enabled and fails when
the function

- has more instructions than its budget, or
- calls or jumps to another function.

Both mean that part of the match stayed out of line. When that happens
the compiler can no longer treat the case pack as constants: it writes
every case to the stack before dispatching, and the cost of the match
grows with the number of cases.

Unlike a timing benchmark this check is deterministic, so it can block a
pull request. Budgets are deliberately loose (a few times the current
count); they exist to catch a match that stopped being inlined, not to
pin exact instruction counts across compiler versions.

Usage:
    check_codegen.py --compiler g++ --include include \\
        --source tests/codegen/literal_dense_64.cpp --max-instructions 40
"""

from __future__ import annotations

import argparse
import re
import subprocess
import sys

DEFAULT_FUNCTION = "ptn_codegen_probe"

# x86 and AArch64 call instructions. Tail calls show up as a jump to a
# mangled C++ symbol and are caught by MANGLED_RE instead.
CALL_RE = re.compile(r"^(call[a-z]*|bl|blr)$")
MANGLED_RE = re.compile(r"\b_{1,2}Z\w+")
LABEL_RE = re.compile(r"^[^\s#;/]\S*:")
COMMENT_RE = re.compile(r"\s*(#|;|//|@).*$")


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


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("--compiler", required=True)
    parser.add_argument("--source", required=True)
    parser.add_argument("--include", action="append", default=[])
    parser.add_argument("--flag", action="append", default=[])
    parser.add_argument("--std", default="c++17")
    parser.add_argument("--opt", default="O2")
    parser.add_argument("--function", default=DEFAULT_FUNCTION)
    parser.add_argument("--max-instructions", type=int, required=True)
    args = parser.parse_args()

    body = function_body(compile_to_assembly(args), args.function)
    if body is None:
        sys.exit(
            f"error: {args.function} not found in the assembly of "
            f"{args.source}"
        )

    found = instructions(body)
    problems = []

    if len(found) > args.max_instructions:
        problems.append(
            f"{len(found)} instructions, budget is "
            f"{args.max_instructions}"
        )

    escapes = [
        text
        for text in found
        if CALL_RE.match(text.split()[0]) or MANGLED_RE.search(text)
    ]
    if escapes:
        problems.append(
            f"{len(escapes)} call(s) to out-of-line code, "
            f"first: {escapes[0][:120]}"
        )

    summary = f"{args.source}: {len(found)} instructions"
    if not problems:
        print(f"{summary} (budget {args.max_instructions}), no calls")
        return 0

    print(f"{summary}")
    for problem in problems:
        print(f"FAIL: {problem}")
    print(
        "The match is no longer fully inlined. See "
        "include/ptn/core/common/inline.hpp."
    )
    print("--- generated code (first 60 instructions) ---")
    for text in found[:60]:
        print(f"    {text[:140]}")
    return 1


if __name__ == "__main__":
    sys.exit(main())
