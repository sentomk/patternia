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
  tests_guard.cpp
  tests_literal_pattern.cpp
  tests_match_flow.cpp
  tests_terminal_semantics.cpp
  tests_type_pattern.cpp
)

target_link_libraries(ptn_tests PRIVATE
  patternia
  GTest::gtest
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(ptn_tests)
