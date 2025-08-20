cmake_minimum_required(VERSION 3.20)

project(patternia
    VERSION 0.1.0
    DESCRIPTION "Match pattern lib for modern C++"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(patternia INTERFACE)

target_include_directories(patternia
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(ptn_tests
    tests/tests_value_pattern.cpp
    tests/tests_relational_pattern.cpp
    tests/tests_dsl_when_then.cpp
)

target_link_libraries(ptn_tests PRIVATE
    patternia
    GTest::gtest
    GTest::gtest_main
)
include(GoogleTest)
gtest_discover_tests(ptn_tests)
