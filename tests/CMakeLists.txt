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
  tests_match_flow.cpp
  tests_public_api.cpp
  tests_terminal_semantics.cpp
  tests_type_pattern.cpp
  tests_unified_match.cpp
)

target_link_libraries(ptn_tests PRIVATE
  patternia
  GTest::gtest
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(ptn_tests)

set(PTN_COMPILE_FAIL_CASES
  compile_fail/chained_when_removed.cpp
  compile_fail/chained_otherwise_removed.cpp
  compile_fail/chained_end_removed.cpp
  compile_fail/match_with_cases_removed.cpp
  compile_fail/lit_v_factory_removed.cpp
  compile_fail/static_on_with_capture.cpp
  compile_fail/on_pipeline_without_wildcard.cpp
)

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
