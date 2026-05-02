# verify_golden.cmake — CMake script mode test for codegen golden snapshot.
# Called by CTest via add_test with -DBENCH_BINARY=... -DGOLDEN_FILE=...
#
# Usage:
#   cmake -DBENCH_BINARY=<path> -DGOLDEN_FILE=<path> -P verify_golden.cmake

cmake_minimum_required(VERSION 3.14)

# ---- Validate inputs ----
if(NOT DEFINED BENCH_BINARY OR NOT BENCH_BINARY)
  message(FATAL_ERROR "BENCH_BINARY not set")
endif()
if(NOT DEFINED GOLDEN_FILE OR NOT GOLDEN_FILE)
  message(FATAL_ERROR "GOLDEN_FILE not set")
endif()

if(NOT EXISTS "${BENCH_BINARY}")
  message(FATAL_ERROR "Benchmark binary not found: ${BENCH_BINARY}")
endif()
if(NOT EXISTS "${GOLDEN_FILE}")
  message(FATAL_ERROR
    "Golden file not found: ${GOLDEN_FILE}\n"
    "Run scripts/refresh_codegen_golden.sh on Linux+Clang18/19 to generate it.")
endif()

# ---- OS guard ----
if(NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
  message(STATUS
    "[codegen golden] Skipping: pinned to Linux+Clang18/19, "
    "current host is ${CMAKE_HOST_SYSTEM_NAME}")
  return()
endif()

# ---- Find objdump ----
find_program(OBJDUMP_EXE NAMES objdump)
if(NOT OBJDUMP_EXE)
  message(FATAL_ERROR "objdump not found in PATH")
endif()

# ---- Extract disassembly ----
set(_tmp_actual "/tmp/ptn_codegen_actual_$ENV{USER}.s")

execute_process(
  COMMAND ${OBJDUMP_EXE} -d --no-show-raw-insn --demangle "${BENCH_BINARY}"
  OUTPUT_VARIABLE _raw_disasm
  ERROR_QUIET
  RESULT_VARIABLE _objdump_rc
)
if(NOT _objdump_rc EQUAL 0)
  message(FATAL_ERROR "objdump failed (exit ${_objdump_rc})")
endif()

# ---- Extract the target function via CMake string ops ----
# Split into lines, find the symbol, collect until blank line after 5+ lines
string(REPLACE "\n" ";" _lines "${_raw_disasm}")

set(_found 0)
set(_line_count 0)
set(_collected "")

foreach(_line IN LISTS _lines)
  if(NOT _found)
    if(_line MATCHES "<BM_PatterniaPipe_LiteralMatch128On_Stateless")
      set(_found 1)
      string(APPEND _collected "${_line}\n")
      set(_line_count 1)
    endif()
  else()
    if(_line STREQUAL "" AND _line_count GREATER 5)
      break()
    endif()
    string(APPEND _collected "${_line}\n")
    math(EXPR _line_count "${_line_count} + 1")
  endif()
endforeach()

if(_collected STREQUAL "")
  message(FATAL_ERROR
    "Could not extract disassembly for BM_PatterniaPipe_LiteralMatch128On_Stateless\n"
    "Ensure the benchmark fixture exists in the binary.")
endif()

# ---- Normalize: strip addresses ----
string(REGEX REPLACE "0x[0-9a-f]+" "<addr>" _normalized "${_collected}")
string(REGEX REPLACE " [0-9a-f]+ <" " <addr><" _normalized "${_normalized}")
string(REGEX REPLACE "<[^>]+>" "<L>" _normalized "${_normalized}")

# ---- Write actual to temp file ----
file(WRITE "${_tmp_actual}" "${_normalized}")

# ---- Diff against golden ----
find_program(DIFF_EXE NAMES diff)
if(NOT DIFF_EXE)
  message(FATAL_ERROR "diff not found in PATH")
endif()

execute_process(
  COMMAND ${DIFF_EXE} -u "${GOLDEN_FILE}" "${_tmp_actual}"
  RESULT_VARIABLE _diff_rc
  OUTPUT_VARIABLE _diff_output
)

if(NOT _diff_rc EQUAL 0)
  message(FATAL_ERROR
    "Codegen golden mismatch!\n"
    "${_diff_output}\n"
    "Run scripts/refresh_codegen_golden.sh on Linux+Clang18/19 if the change is intentional.")
endif()

message(STATUS "[codegen golden] PASS — output matches ${GOLDEN_FILE}")
