cmake_minimum_required(VERSION 3.20)
project(patternia
  VERSION 0.3.0
  DESCRIPTION "Header-only pattern matching library for modern C++"
  LANGUAGES CXX
)

# Require C++23 standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --------------------------------------------------
# Interface library target for patternia
# --------------------------------------------------
add_library(patternia INTERFACE)

target_include_directories(patternia
  INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# --------------------------------------------------
# FetchContent for external dependencies
# --------------------------------------------------
include(FetchContent)

# --- GoogleTest for unit testing ---
# Turn off unneeded components before declaration
set(BUILD_GMOCK OFF CACHE BOOL "Disable GoogleMock" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "Disable GTest install" FORCE)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.17.0
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(googletest)

# --- Google Benchmark for performance testing ---
# Disable benchmark's internal tests
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Disable benchmark tests" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "Disable benchmark GTest tests" FORCE)

FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.9.4
)
FetchContent_MakeAvailable(benchmark)

# --------------------------------------------------
# Options to enable/disable tests and benchmarks
# --------------------------------------------------
option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_BENCHMARKS "Build benchmarks" OFF)

# Find Threads package (required by benchmark)
find_package(Threads REQUIRED)

# --------------------------------------------------
# Unit tests target
# --------------------------------------------------
if(BUILD_TESTS)
  enable_testing()
  add_executable(ptn_tests
    tests/tests_value_pattern.cpp
    tests/tests_relational_pattern.cpp
  )
  target_link_libraries(ptn_tests PRIVATE
    patternia
    GTest::gtest
    GTest::gtest_main
  )
  include(GoogleTest)
  gtest_discover_tests(ptn_tests)
endif()

# --------------------------------------------------
# Benchmark target
# --------------------------------------------------
if(BUILD_BENCHMARKS)
  add_executable(bench_patternia
    benchmarks/bench_patternia.cpp
  )
  target_link_libraries(bench_patternia PRIVATE
    patternia
    benchmark::benchmark
    Threads::Threads
  )

  # Optimize for performance tests
  target_compile_options(bench_patternia PRIVATE -O3 -march=native)
endif()

# --------------------------------------------------
# Installation rules (optional)
# --------------------------------------------------
install(TARGETS patternia EXPORT patterniaTargets)
install(DIRECTORY include/ DESTINATION include)

install(EXPORT patterniaTargets
  FILE patterniaConfig.cmake
  NAMESPACE patternia::
  DESTINATION lib/cmake/patternia
)
