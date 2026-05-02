include(FetchContent)

find_package(GTest CONFIG QUIET)

if(NOT TARGET GTest::gtest)
  set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
    GIT_SHALLOW TRUE
  )
  FetchContent_MakeAvailable(googletest)
endif()

add_executable(ptn_tests
  tests_destructure.cpp
  tests_guard.cpp
  tests_literal_pattern.cpp
  tests_combinator.cpp
  tests_match_flow.cpp
  tests_pipeline_match.cpp
  tests_pred_pattern.cpp
  tests_public_api.cpp
  tests_terminal_semantics.cpp
  tests_type_pattern.cpp
  test_on_auto_cache_multi_tu.cpp
  test_on_auto_cache_multi_tu_helper.cpp
  test_on_auto_cache_identity.cpp
)

target_link_libraries(ptn_tests PRIVATE
  patternia
  GTest::gtest
  GTest::gtest_main
)

if(PTN_BUILD_TESTING_HOOKS)
  target_compile_definitions(ptn_tests PRIVATE PTN_TESTING)
endif()

include(GoogleTest)
gtest_discover_tests(ptn_tests)

set(PTN_COMPILE_FAIL_CASES
  compile_fail/chained_when_removed.cpp
  compile_fail/chained_otherwise_removed.cpp
  compile_fail/chained_end_removed.cpp
  compile_fail/cases_factory_removed.cpp
  compile_fail/direct_match_removed.cpp
  compile_fail/bind_function_removed.cpp
  compile_fail/guard_placeholder_alias_removed.cpp
  compile_fail/has_factory_removed.cpp
  compile_fail/lit_v_factory_removed.cpp
  compile_fail/lit_factory_removed.cpp
  compile_fail/any_requires_subpattern.cpp
  compile_fail/all_requires_subpattern.cpp
  compile_fail/any_requires_pattern_args.cpp
  compile_fail/all_requires_pattern_args.cpp
  compile_fail/static_on_with_capture.cpp
  compile_fail/on_pipeline_without_wildcard.cpp
)

if(CMAKE_CXX_STANDARD LESS 20)
  list(APPEND PTN_COMPILE_FAIL_CASES
    compile_fail/val_float_requires_cpp20.cpp
  )
endif()

function(ptn_add_compile_fail_test SOURCE_REL)
  get_filename_component(_name "${SOURCE_REL}" NAME_WE)
  set(_source "${CMAKE_CURRENT_LIST_DIR}/${SOURCE_REL}")
  set(_obj "${CMAKE_CURRENT_BINARY_DIR}/compile_fail/${_name}.o")

  add_test(
    NAME "compile_fail.${_name}"
    COMMAND
      ${CMAKE_COMMAND}
      -DCXX_COMPILER=${CMAKE_CXX_COMPILER}
      -DCXX_COMPILER_ID=${CMAKE_CXX_COMPILER_ID}
      -DCXX_SIMULATE_ID=${CMAKE_CXX_SIMULATE_ID}
      -DCXX_STANDARD=${CMAKE_CXX_STANDARD}
      -DINCLUDE_DIR=${PROJECT_SOURCE_DIR}/include
      -DSOURCE_FILE=${_source}
      -DOBJECT_FILE=${_obj}
      -P ${CMAKE_CURRENT_LIST_DIR}/compile_fail/run_compile_fail.cmake
  )
endfunction()

foreach(_case IN LISTS PTN_COMPILE_FAIL_CASES)
  ptn_add_compile_fail_test("${_case}")
endforeach()

# ========== Codegen golden snapshot test ==========
# Gated behind PTN_CODEGEN_GOLDEN_ENABLED (OFF by default).
# Pinned to Linux+Clang18/19; gracefully skips on other platforms via the
# cmake script (verify_golden.cmake checks CMAKE_HOST_SYSTEM_NAME).
if(PTN_CODEGEN_GOLDEN_ENABLED)
  if(NOT PTN_BUILD_BENCHMARKS)
    message(WARNING
      "[patternia] PTN_CODEGEN_GOLDEN_ENABLED=ON requires PTN_BUILD_BENCHMARKS=ON "
      "to build ptn_bench_lit. The codegen golden test will be registered but may "
      "fail if the binary is absent.")
  endif()
  # Note: ptn_bench_lit is defined in bench/ which is added after tests/.
  # We use a deferred generator expression $<TARGET_FILE:ptn_bench_lit> so the
  # path is resolved at build time, not configure time.
  add_test(
    NAME codegen.lm128_on_stateless_golden
    COMMAND
      ${CMAKE_COMMAND}
      -DBENCH_BINARY=${CMAKE_BINARY_DIR}/bench/ptn_bench_lit
      -DGOLDEN_FILE=${CMAKE_CURRENT_SOURCE_DIR}/codegen/lm128_on.s.expected
      -P ${CMAKE_CURRENT_SOURCE_DIR}/codegen/verify_golden.cmake
  )
  set_tests_properties(codegen.lm128_on_stateless_golden PROPERTIES
    LABELS "codegen;golden"
  )
endif()
