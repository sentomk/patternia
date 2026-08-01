include(FetchContent)
find_package(Threads REQUIRED)

# ── literal-match compile-time TU helper ──────────────────────
# Each TU instantiates match(...) | on( lit(0)>>0, lit(1)>>1, ... )
# at compile time (in a constexpr function + static_assert chain),
# so the TU itself exercises the template machinery without any
# runtime dependency on Google Benchmark.

function(_ptn_ct_add_lit_tu N)
  # Generate into the build tree, not the source tree: writing to
  # CMAKE_CURRENT_SOURCE_DIR dirtied the working tree on every
  # configure, and the generated files must not be committed.
  set(_tufile "${CMAKE_CURRENT_BINARY_DIR}/ct_bench_lit_${N}.cpp")
  file(WRITE "${_tufile}"
    "// compile-time literal-match benchmark – forces full instantiation\n"
    "// of the match-dispatch machinery with ${N} literal cases.\n\n"
    "#include \"ptn/patternia.hpp\"\n\n"
  )

  # constexpr function body
  file(APPEND "${_tufile}"
    "constexpr int ct_lit_${N}(int v) noexcept {\n"
    "  using namespace ptn;\n"
    "  return match(v) | on(\n"
  )
  set(_cases "")
  math(EXPR _last "${N} - 1")
  foreach(_i RANGE 0 ${_last} 1)
    string(APPEND _cases "      ptn::lit(${_i}) >> ${_i},\n")
  endforeach()
  string(APPEND _cases "      _ >> 0);\n")
  file(APPEND "${_tufile}" "${_cases}")
  file(APPEND "${_tufile}" "}\n\n")

  # static_assert chain to force constexpr evaluation
  math(EXPR _half "${N} / 2")
  file(APPEND "${_tufile}"
    "static_assert(ct_lit_${N}(0) == 0, \"\");\n"
    "static_assert(ct_lit_${N}(${_half}) == ${_half}, \"\");\n"
    "static_assert(ct_lit_${N}(${_last}) == ${_last}, \"\");\n"
    "static_assert(ct_lit_${N}(-1) == 0, \"\");\n"
    "static_assert(ct_lit_${N}(${N}) == 0, \"\");\n"
  )

  # object library – compile-only, no link
  add_library(ptn_ct_lit_${N} OBJECT "${_tufile}")
  target_link_libraries(ptn_ct_lit_${N} PRIVATE patternia Threads::Threads)
  target_compile_features(ptn_ct_lit_${N} PRIVATE cxx_std_17)

  if(MSVC)
    target_compile_options(ptn_ct_lit_${N} PRIVATE /O2)
  else()
    target_compile_options(ptn_ct_lit_${N} PRIVATE -O3)
  endif()
endfunction()

# ── generate literal-match TUs for 8/16/32/64/128 ────────────
_ptn_ct_add_lit_tu(8)
_ptn_ct_add_lit_tu(16)
_ptn_ct_add_lit_tu(32)
_ptn_ct_add_lit_tu(64)
_ptn_ct_add_lit_tu(128)

# ── variant compile-time bench ────────────────────────────────
# Forces instantiation of std::variant alt-matching with 32 alts.
add_library(ptn_ct_variant_32 OBJECT ct_bench_variant_32.cpp)
target_link_libraries(ptn_ct_variant_32 PRIVATE patternia Threads::Threads)
target_compile_features(ptn_ct_variant_32 PRIVATE cxx_std_17)
if(MSVC)
  target_compile_options(ptn_ct_variant_32 PRIVATE /O2)
else()
  target_compile_options(ptn_ct_variant_32 PRIVATE -O3)
endif()

# ── compound compile-time bench ───────────────────────────────
# Combines multiple pattern types (literal, variant, predicate)
# in a single TU to measure worst-case instantiation cost.
add_library(ptn_ct_compound OBJECT ct_bench_compound.cpp)
target_link_libraries(ptn_ct_compound PRIVATE patternia Threads::Threads)
target_compile_features(ptn_ct_compound PRIVATE cxx_std_17)
if(MSVC)
  target_compile_options(ptn_ct_compound PRIVATE /O2)
else()
  target_compile_options(ptn_ct_compound PRIVATE -O3)
endif()

# ── runtime-dense compile-time bench ───────────────────────────
# 15 sparse literal values (density ≈ 0.15) to exercise the
# literal_runtime_dense dispatch tier at compile time.
add_library(ptn_ct_lit_rdense OBJECT ct_bench_lit_rdense.cpp)
target_link_libraries(ptn_ct_lit_rdense PRIVATE patternia Threads::Threads)
target_compile_features(ptn_ct_lit_rdense PRIVATE cxx_std_17)
if(MSVC)
  target_compile_options(ptn_ct_lit_rdense PRIVATE /O2)
else()
  target_compile_options(ptn_ct_lit_rdense PRIVATE -O3)
endif()

# ── aggregate convenience target ──────────────────────────────
# `cmake --build build --target ptn_bench_ct` compiles every TU.
add_custom_target(ptn_bench_ct)
add_dependencies(ptn_bench_ct
  ptn_ct_lit_8   ptn_ct_lit_16  ptn_ct_lit_32
  ptn_ct_lit_64  ptn_ct_lit_128
  ptn_ct_variant_32
  ptn_ct_compound
  ptn_ct_lit_rdense
)
