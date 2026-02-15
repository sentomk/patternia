include(FetchContent)
find_package(Threads REQUIRED)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.9.4
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(benchmark)

add_executable(ptn_bench
  bench_match_variant.cpp
)

target_link_libraries(ptn_bench PRIVATE
  patternia
  benchmark::benchmark
  benchmark::benchmark_main
  Threads::Threads
)

target_compile_features(ptn_bench PRIVATE cxx_std_17)

if(MSVC)
  target_compile_options(ptn_bench PRIVATE /O2)
else()
  target_compile_options(ptn_bench PRIVATE -O3)
endif()

