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
  tests_member_names.cpp
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
