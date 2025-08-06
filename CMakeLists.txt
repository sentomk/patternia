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

add_executable(ptn_tests
    tests/tests_ptn.cpp
)

target_link_libraries(ptn_tests PRIVATE patternia)

enable_testing()
add_test(
    NAME PatterniaTests
    COMMAND ptn_tests
)
