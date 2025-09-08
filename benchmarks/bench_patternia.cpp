#include "benchmark/benchmark.h"
#include "ptn/patternia.hpp"

using namespace ptn;

static void BM_IfElse(benchmark::State &state) {
  int x = static_cast<int>(state.range(0));
  for (auto _ : state) {
    int result;
    if (x < 0)
      result = -1;
    else if (x <= 42)
      result = 0;
    else
      result = 1;
    benchmark::DoNotOptimize(result);
  }
}
BENCHMARK(BM_IfElse)->Arg(-10)->Arg(0)->Arg(42)->Arg(100);

static void BM_Switch(benchmark::State &state) {
  int x = static_cast<int>(state.range(0));
  for (auto _ : state) {
    int result;
    switch (x) {
    case 0:
      result = 0;
      break;
    case 42:
      result = 1;
      break;
    default:
      result = (x < 0 ? -1 : 2);
    }
    benchmark::DoNotOptimize(result);
  }
}
BENCHMARK(BM_Switch)->Arg(-10)->Arg(0)->Arg(42)->Arg(100);

static void BM_Patternia(benchmark::State &state) {
  int x = static_cast<int>(state.range(0));
  for (auto _ : state) {
    auto result = match(x)
                      .with(lt(0), [] { return -1; })
                      .with(le(42), [] { return 0; })
                      .otherwise([] { return 1; });
    benchmark::DoNotOptimize(result);
  }
}
BENCHMARK(BM_Patternia)->Arg(-10)->Arg(0)->Arg(42)->Arg(100);

BENCHMARK_MAIN();