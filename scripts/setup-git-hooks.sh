#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

git config core.hooksPath .githooks
echo "Configured git hooks path: .githooks"
echo "pre-commit hook will format staged C/C++ files with the pinned clang-format (tools/requirements.txt)."
