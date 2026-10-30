#!/usr/bin/env python3
"""Classify the files a pull request changes so CI can skip jobs.

Writes two outputs to $GITHUB_OUTPUT (and prints them):

  code=true|false   some changed file can affect a build, a test or a
                    CI result. False only when every changed file is
                    documentation, agent instructions or packaging
                    metadata that no CI job builds or reads.
  perf=true|false   some changed file can affect benchmark results.

Only pull_request events are filtered. Pushes, scheduled runs and manual
runs always report true, and so does any case where the changed files
cannot be determined, so a mistake here costs CI minutes, never coverage.

The checkout must be the default pull_request one (the PR merged into its
base) with fetch-depth >= 2: the first parent of HEAD is the base, so
`git diff HEAD^1 HEAD` lists exactly what the PR changes.

Usage:
  ci_changes.py                  # inside GitHub Actions
  ci_changes.py FILE [FILE...]   # classify the given paths (local check)
"""

from __future__ import annotations

import os
import subprocess
import sys

# Files and directories no CI job builds, tests or reads.
NON_CODE_PREFIXES = (
    "docs/",
    "ai/",
    ".claude/",
    "packaging/",
    ".github/ISSUE_TEMPLATE/",
)
NON_CODE_FILES = frozenset(
    {
        "README.md",
        "CONTRIBUTING.md",
        "AGENTS.md",
        "SECURITY.md",
        "CODE_OF_CONDUCT.md",
        "LICENSE",
        "mkdocs.yml",
        ".github/PULL_REQUEST_TEMPLATE.md",
    }
)

# Files that can change benchmark numbers or the benchmark job itself.
PERF_PREFIXES = (
    "include/",
    "bench/",
    "cmake/",
    "scripts/bench_",
)
PERF_FILES = frozenset(
    {
        "CMakeLists.txt",
        "tools/requirements-bench.txt",
        ".github/workflows/ci-pr.yml",
    }
)


def is_code(path: str) -> bool:
    return path not in NON_CODE_FILES and not path.startswith(NON_CODE_PREFIXES)


def is_perf(path: str) -> bool:
    return path in PERF_FILES or path.startswith(PERF_PREFIXES)


def changed_files() -> list[str] | None:
    try:
        out = subprocess.run(
            ["git", "diff", "--no-renames", "--name-only", "HEAD^1", "HEAD"],
            check=True,
            capture_output=True,
            text=True,
        ).stdout
    except (subprocess.CalledProcessError, OSError):
        return None
    files = [line for line in out.splitlines() if line]
    return files or None


def classify(files: list[str] | None) -> tuple[bool, bool]:
    if files is None:
        return True, True
    return any(map(is_code, files)), any(map(is_perf, files))


def main() -> int:
    if len(sys.argv) > 1:
        files: list[str] | None = sys.argv[1:]
    elif os.environ.get("GITHUB_EVENT_NAME") == "pull_request":
        files = changed_files()
        if files is None:
            print("could not list the changed files; running everything")
    else:
        files = None

    code, perf = classify(files)

    if files is not None:
        print(f"{len(files)} changed file(s):")
        for f in files:
            print(f"  {f}")
    print(f"code={str(code).lower()}")
    print(f"perf={str(perf).lower()}")

    out = os.environ.get("GITHUB_OUTPUT")
    if out:
        with open(out, "a", encoding="utf-8") as fh:
            fh.write(f"code={str(code).lower()}\n")
            fh.write(f"perf={str(perf).lower()}\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
