#include <benchmark/benchmark.h>
#include "ptn/patternia.hpp"
#include <random>
#include <vector>

using namespace ptn;
using namespace ptn::dsl;

static std::vector<int> generate_int_data(size_t n) {
  std::mt19937                       rng(123);
  std::uniform_int_distribution<int> dist(0, 100);
  std::vector<int>                   data;
  data.reserve(n);
  for (size_t i = 0; i < n; ++i)
    data.push_back(dist(rng));
  return data;
}

// Hand-written if-else chain
static int ifelse_match(int x) {
  if (x < 10)
    return 0;
  else if (x < 20)
    return 1;
  else if (x < 30)
    return 2;
  else if (x < 40)
    return 3;
  else if (x < 50)
    return 4;
  else
    return 5;
}

// Switch-case implementation
static int switch_match(int x) {
  switch (x / 10) {
  case 0:
    return 0;
  case 1:
    return 1;
  case 2:
    return 2;
  case 3:
    return 3;
  case 4:
    return 4;
  default:
    return 5;
  }
}

// Patternia match using DSL sugar (.when(pattern >> handler))
static int patternia_match(int x) {
  return match(x)
      .when(lt(10) >> 0)
      .when(lt(20) >> 1)
      .when(lt(30) >> 2)
      .when(lt(40) >> 3)
      .when(lt(50) >> 4)
      .otherwise(5);
}

// Benchmark for if-else
static void BM_IfElse(benchmark::State &state) {
  auto data = generate_int_data(state.range(0));
  int  sum  = 0;
  for (auto _ : state) {
    for (auto v : data)
      sum += ifelse_match(v);
  }
  benchmark::DoNotOptimize(sum);
}
BENCHMARK(BM_IfElse)->RangeMultiplier(10)->Range(1000, 1000000);

// Benchmark for switch
static void BM_Switch(benchmark::State &state) {
  auto data = generate_int_data(state.range(0));
  int  sum  = 0;
  for (auto _ : state) {
    for (auto v : data)
      sum += switch_match(v);
  }
  benchmark::DoNotOptimize(sum);
}
BENCHMARK(BM_Switch)->RangeMultiplier(10)->Range(1000, 1000000);

// Benchmark for patternia
static void BM_Patternia(benchmark::State &state) {
  auto data = generate_int_data(state.range(0));
  int  sum  = 0;
  for (auto _ : state) {
    for (auto v : data)
      sum += patternia_match(v);
  }
  benchmark::DoNotOptimize(sum);
}
BENCHMARK(BM_Patternia)->RangeMultiplier(10)->Range(1000, 1000000);
