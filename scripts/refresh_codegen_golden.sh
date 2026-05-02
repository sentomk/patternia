#!/usr/bin/env bash
set -euo pipefail

# Refresh codegen golden snapshot for LM128_On_Stateless benchmark.
# This script rebuilds the benchmark with Release mode and extracts normalized
# disassembly for the BM_PatterniaPipe_LiteralMatch128On_Stateless symbol.

# OS guard: codegen golden is pinned to Linux+Clang18
if [[ "$OSTYPE" != "linux-gnu"* ]]; then
  echo "Codegen golden is pinned to Linux+Clang18; current OS is $OSTYPE — refusing." >&2
  exit 2
fi

# Compiler version check: clang++ 18 or 19 required
if ! clang++ --version 2>/dev/null | grep -qE 'clang version 1[89]'; then
  echo "ERROR: clang++ 18/19 required" >&2
  exit 1
fi

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

# Build step: configure and build with benchmarks enabled
echo "Building LM128_On_Stateless benchmark..."
cmake -B build-codegen \
  -DCMAKE_BUILD_TYPE=Release \
  -DPTN_BUILD_BENCHMARKS=ON \
  -DCMAKE_CXX_COMPILER=clang++ \
  >/dev/null 2>&1 || {
  echo "ERROR: CMake configuration failed" >&2
  exit 1
}

cmake --build build-codegen --target ptn_bench_lit -j >/dev/null 2>&1 || {
  echo "ERROR: Build failed" >&2
  exit 1
}

# Disassembly step: extract the benchmark symbol
echo "Extracting disassembly for BM_PatterniaPipe_LiteralMatch128On_Stateless..."
bench_binary="build-codegen/bench/ptn_bench_lit"

if [[ ! -f "$bench_binary" ]]; then
  echo "ERROR: Benchmark binary not found at $bench_binary" >&2
  exit 1
fi

# Try GNU objdump --disassemble=<symbol> first (more precise)
if objdump --help 2>/dev/null | grep -q "disassemble="; then
  disasm=$(objdump -d --no-show-raw-insn --demangle \
    --disassemble='BM_PatterniaPipe_LiteralMatch128On_Stateless' \
    "$bench_binary" 2>/dev/null || true)
else
  # Fallback: use awk to extract symbol and follow until blank line
  disasm=$(objdump -d --no-show-raw-insn --demangle "$bench_binary" 2>/dev/null | \
    awk '/<BM_PatterniaPipe_LiteralMatch128On_Stateless/{found=1} found{print; if(/^$/ && found>5) exit}' || true)
fi

if [[ -z "$disasm" ]]; then
  echo "ERROR: Could not extract disassembly for BM_PatterniaPipe_LiteralMatch128On_Stateless" >&2
  echo "       (Benchmark fixture may not exist yet; this is expected during development)" >&2
  exit 1
fi

# Normalization: strip addresses and branch targets
normalized=$(echo "$disasm" | sed -E \
  's/0x[0-9a-f]+/<addr>/g; s/ [0-9a-f]+ </ <addr></g; s/<[^>]+>/<L>/g')

# Ensure output directory exists
mkdir -p tests/codegen

# Write output
output_file="tests/codegen/lm128_on.s.expected"
echo "$normalized" > "$output_file"

echo "Golden written to $output_file"
