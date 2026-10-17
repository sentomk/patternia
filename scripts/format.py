#!/usr/bin/env python3
"""Check or fix clang-format style with the pinned clang-format.

usage: scripts/format.py [--check | --fix] [scope]

scope (default: files changed against the merge base with origin/main,
including uncommitted and untracked files):
  --staged         files staged for commit
  --all            every tracked C/C++ file
  --base REV       files changed between REV and the working tree
  --base REV --head REV
                   files changed between two commits (used by CI)

--check (the default) prints a diff for each file that is not clean and
exits 1. --fix rewrites the files in place; with --staged it also
re-stages them, which is what the pre-commit hook does.
"""

from __future__ import annotations

import argparse
import difflib
import subprocess
import sys
from pathlib import Path
from typing import List, Optional

repository_root = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(repository_root / "tools"))
import envsetup  # noqa: E402

EXTENSIONS = (".h", ".hpp", ".hh", ".hxx", ".c", ".cc", ".cpp", ".cxx", ".ipp")
ZERO_SHA = "0" * 40


def git(*args: str, check: bool = True) -> str:
    result = subprocess.run(
        ["git", *args],
        cwd=repository_root,
        check=check,
        stdout=subprocess.PIPE,
        stderr=subprocess.DEVNULL,
        text=True,
    )
    return result.stdout if result.returncode == 0 else ""


def lines(output: str) -> List[str]:
    return [line for line in output.splitlines() if line]


def merge_base() -> str:
    for ref in ("origin/main", "main"):
        base = git("merge-base", "HEAD", ref, check=False).strip()
        if base:
            return base
    return "HEAD"


def candidate_files(args: argparse.Namespace) -> List[str]:
    if args.staged:
        return lines(git("diff", "--cached", "--name-only", "--diff-filter=ACMR"))
    if args.all:
        return lines(git("ls-files"))

    base: Optional[str] = args.base
    if base == ZERO_SHA:
        # A brand-new branch push reports an all-zero "before" sha.
        base = git("rev-list", "--max-parents=0", args.head or "HEAD").splitlines()[-1]
    if base is None:
        files = lines(git("diff", "--name-only", "--diff-filter=ACMR", merge_base()))
        files += lines(git("ls-files", "--others", "--exclude-standard"))
        return files
    if args.head:
        return lines(git("diff", "--name-only", "--diff-filter=ACMR", base, args.head))
    return lines(git("diff", "--name-only", "--diff-filter=ACMR", base))


def select(files: List[str]) -> List[str]:
    seen = []
    for name in files:
        if name.endswith(EXTENSIONS) and name not in seen and (repository_root / name).is_file():
            seen.append(name)
    return seen


def formatted(clang_format: Path, name: str) -> bytes:
    return subprocess.run(
        [str(clang_format), "-style=file", name],
        cwd=repository_root,
        check=True,
        stdout=subprocess.PIPE,
    ).stdout


def show_diff(name: str, before: bytes, after: bytes) -> None:
    diff = difflib.unified_diff(
        before.decode("utf-8", "replace").splitlines(keepends=True),
        after.decode("utf-8", "replace").splitlines(keepends=True),
        fromfile=f"a/{name}",
        tofile=f"b/{name}",
    )
    sys.stdout.writelines(diff)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--check", action="store_true", help="report files that need formatting (default)")
    mode.add_argument("--fix", action="store_true", help="rewrite files in place")
    scope = parser.add_mutually_exclusive_group()
    scope.add_argument("--staged", action="store_true")
    scope.add_argument("--all", action="store_true")
    scope.add_argument("--base")
    parser.add_argument("--head", help="with --base: compare against this commit instead of the working tree")
    args = parser.parse_args()
    if args.head and not args.base:
        parser.error("--head needs --base")

    files = select(candidate_files(args))
    if not files:
        print("No C/C++ files to check.")
        return 0

    clang_format = envsetup.setup_environment()
    dirty = []
    for name in files:
        path = repository_root / name
        before = path.read_bytes()
        after = formatted(clang_format, name)
        if before == after:
            continue
        dirty.append(name)
        if args.fix:
            path.write_bytes(after)
            if args.staged:
                git("add", "--", name)
        else:
            show_diff(name, before, after)

    if not dirty:
        print(f"All {len(files)} C/C++ files are clang-format clean.")
        return 0
    if args.fix:
        print(f"Reformatted {len(dirty)} file(s):")
        print("\n".join(f"  {name}" for name in dirty))
        return 0
    print(f"\n{len(dirty)} file(s) are not clang-format clean:", file=sys.stderr)
    print("\n".join(f"  {name}" for name in dirty), file=sys.stderr)
    print("Fix them with: python3 scripts/format.py --fix", file=sys.stderr)
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
