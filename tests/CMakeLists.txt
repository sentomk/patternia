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
  tests_canonical_patterns.cpp
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
  tests_named_placeholder.cpp
  tests_handler_forms.cpp
  tests_expression_handler.cpp
  tests_dispatch_fallback.cpp
  tests_dispatch_metadata.cpp
  tests_move_matching.cpp
  tests_member_names.cpp
  tests_header_first.cpp
)

target_link_libraries(ptn_tests PRIVATE
  patternia
  GTest::gtest
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(ptn_tests)

# Exercise static structured bindings even when the main suite is
# built with the project's C++17 default.
if("cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
  add_executable(ptn_member_names_cpp20 tests_member_names.cpp)
  target_compile_features(ptn_member_names_cpp20 PRIVATE cxx_std_20)
  target_link_libraries(ptn_member_names_cpp20 PRIVATE
    patternia
    GTest::gtest
    GTest::gtest_main
  )
  gtest_discover_tests(ptn_member_names_cpp20 TEST_PREFIX "cxx20.")
endif()

set(PTN_COMPILE_FAIL_CASES
  compile_fail/owned_guard_cannot_consume.cpp
  compile_fail/owned_mixed_void_results.cpp
  compile_fail/owned_incompatible_results.cpp
  compile_fail/owned_const_binding_cannot_mutate.cpp
  compile_fail/owned_lvalue_pack_rvalue_handler.cpp
  compile_fail/owned_lvalue_pack_move_only_value.cpp
  compile_fail/borrowed_subject_cannot_move.cpp
  compile_fail/on_builtin_shift.cpp
  compile_fail/on_builtin_shift_stored.cpp
  compile_fail/on_non_case.cpp
  compile_fail/on_non_case_lvalue.cpp
  compile_fail/on_non_case_validation.cpp
  compile_fail/expression_handler_requires_binding.cpp
  compile_fail/expression_handler_result_requires_binding.cpp
  compile_fail/expression_handler_foreign_member.cpp
  compile_fail/expression_handler_non_structural_member.cpp
  compile_fail/expression_handler_out_of_range.cpp
  compile_fail/expression_handler_comparison_requires_binding.cpp
  compile_fail/chained_when_removed.cpp
  compile_fail/chained_otherwise_removed.cpp
  compile_fail/chained_end_removed.cpp
  compile_fail/cases_factory_removed.cpp
  compile_fail/direct_match_removed.cpp
  compile_fail/bind_function_removed.cpp
  compile_fail/guard_placeholder_alias_removed.cpp
  compile_fail/zero_placeholder_removed.cpp
  compile_fail/indexed_placeholder_removed.cpp
  compile_fail/ptn_let_removed.cpp
  compile_fail/ptn_where_removed.cpp
  compile_fail/double_wildcard_alias_removed.cpp
  compile_fail/has_factory_removed.cpp
  compile_fail/lit_v_factory_removed.cpp
  compile_fail/lit_factory_removed.cpp
  compile_fail/any_requires_subpattern.cpp
  compile_fail/all_requires_subpattern.cpp
  compile_fail/any_requires_pattern_args.cpp
  compile_fail/all_requires_pattern_args.cpp
  compile_fail/static_on_with_capture.cpp
  compile_fail/on_pipeline_without_wildcard.cpp
  compile_fail/on_pipeline_guarded_dollar_without_fallback.cpp
  compile_fail/on_pipeline_guarded_dollar_call_without_fallback.cpp
  compile_fail/on_pipeline_subpattern_without_fallback.cpp
  compile_fail/on_pipeline_dollar_not_last.cpp
  compile_fail/on_pipeline_wildcard_before_dollar.cpp
  compile_fail/val_requires_compile_time_constant.cpp
  compile_fail/member_placeholder_not_in_pattern.cpp
  compile_fail/member_placeholder_non_structural.cpp
  compile_fail/member_names_foreign_pattern.cpp
  compile_fail/member_names_ignored_member.cpp
  compile_fail/member_names_non_structural.cpp
  compile_fail/ptn_bind_unknown_member.cpp
  compile_fail/member_names_local_ptn_on.cpp
  compile_fail/has_get_out_of_range.cpp
  compile_fail/has_get_empty.cpp
  compile_fail/has_get_ignored.cpp
  compile_fail/has_tuple_element_out_of_range.cpp
  compile_fail/has_tuple_element_empty.cpp
  compile_fail/has_tuple_element_ignored.cpp
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

# Codegen gate: each probe holds one match that must compile down to
# inlined, keyed code; see scripts/check_codegen.py. The probes are
# compiled with optimization here, independent of the build type.
#
# Every probe is checked for calls and stack use: a match that stops
# being fully inlined writes its whole case pack to the stack on every
# call. Options after the name add limits:
#
#   max-branches=N      conditional branches. A keyed match has only
#                       its bounds check; a chain or tree of
#                       comparisons has one branch per step.
#   max-instructions=N  only where code size does not grow with the
#                       number of cases: a dense match may legitimately
#                       become a jump table with a few instructions per
#                       case.
#   reference=F         no more instructions than function F in the
#                       same file, plus a small slack. Compares two
#                       spellings of one match under each compiler's
#                       own switch lowering.
#   skip-libcxx         libc++ re-tests the active alternative inside
#                       each arm of a binding variant match, which the
#                       optimizer does not fold. Tracked as separate
#                       work; the probe would only report that.
set(PTN_CODEGEN_PROBES
  literal_dense_16:max-branches=4
  literal_dense_64:max-branches=4
  literal_dense_64_owned:max-branches=4
  literal_runtime_8:max-instructions=60
  guard_chain:max-instructions=45
  structural_guard:max-instructions=45
  variant_simple_16:max-branches=3
  variant_bind_16:max-branches=3:skip-libcxx
  variant_cached_16:max-branches=3:reference=ptn_codegen_reference
  literal_cached_64:max-branches=4:reference=ptn_codegen_reference
)

find_package(Python3 COMPONENTS Interpreter QUIET)

if(Python3_Interpreter_FOUND
   AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang"
   AND NOT MSVC
   AND NOT CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC"
   AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|aarch64|arm64")
  include(CheckCXXSourceCompiles)
  check_cxx_source_compiles("
    #include <cstddef>
    #ifndef _LIBCPP_VERSION
    #error not libc++
    #endif
    int main() { return 0; }
  " PTN_TESTS_USE_LIBCXX)

  foreach(_probe IN LISTS PTN_CODEGEN_PROBES)
    string(REPLACE ":" ";" _probe_fields "${_probe}")
    list(GET _probe_fields 0 _probe_name)
    list(REMOVE_AT _probe_fields 0)

    set(_probe_args "")
    set(_probe_skip OFF)
    foreach(_probe_option IN LISTS _probe_fields)
      if(_probe_option STREQUAL "skip-libcxx")
        if(PTN_TESTS_USE_LIBCXX)
          set(_probe_skip ON)
        endif()
      else()
        string(REPLACE "=" ";" _probe_pair "${_probe_option}")
        list(GET _probe_pair 0 _probe_key)
        list(GET _probe_pair 1 _probe_value)
        list(APPEND _probe_args "--${_probe_key}" "${_probe_value}")
      endif()
    endforeach()

    if(NOT _probe_skip)
      add_test(
        NAME "codegen.${_probe_name}"
        COMMAND
          ${Python3_EXECUTABLE}
          ${PROJECT_SOURCE_DIR}/scripts/check_codegen.py
          --compiler ${CMAKE_CXX_COMPILER}
          --include ${PROJECT_SOURCE_DIR}/include
          --source ${CMAKE_CURRENT_LIST_DIR}/codegen/${_probe_name}.cpp
          ${_probe_args}
      )
    endif()
  endforeach()
endif()
