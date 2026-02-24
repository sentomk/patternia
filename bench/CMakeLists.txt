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
  bench_pipe_vs_std.cpp
)

add_executable(ptn_bench_lit
  bench_ptn_lit.cpp
)


add_executable(ptn_bench_evolution
  bench_pipe_vs_chain.cpp
)

foreach(_ptn_bench_target IN ITEMS ptn_bench ptn_bench_lit ptn_bench_evolution)
  target_link_libraries(${_ptn_bench_target} PRIVATE
    patternia
    benchmark::benchmark
    benchmark::benchmark_main
    Threads::Threads
  )

  target_compile_features(${_ptn_bench_target} PRIVATE cxx_std_17)

  if(MSVC)
    target_compile_options(${_ptn_bench_target} PRIVATE /O2)
  else()
    target_compile_options(${_ptn_bench_target} PRIVATE -O3)
  endif()
endforeach()

