#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

git config core.hooksPath .githooks
echo "Configured git hooks path: .githooks"
echo "pre-commit hook will run clang-format on staged C/C++ files."
