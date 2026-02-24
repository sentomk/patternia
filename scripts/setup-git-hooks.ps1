$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$hooksDir = Join-Path $repoRoot ".githooks"

if (!(Test-Path $hooksDir)) {
  throw "Hooks directory not found: $hooksDir"
}

git config core.hooksPath .githooks
Write-Host "Configured git hooks path: .githooks"
Write-Host "pre-commit hook will run clang-format on staged C/C++ files."
