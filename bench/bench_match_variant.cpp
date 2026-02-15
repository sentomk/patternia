#include <string>
#include <type_traits>
#include <variant>

#include <benchmark/benchmark.h>

#include "ptn/patternia.hpp"

namespace {
  using V = std::variant<int, std::string>;

  static int patternia_route(const V &v) {
    using namespace ptn;
    using ptn::pat::type::is;

    return match(v)
        .when(is<int>() >> 1)
        .when(is<std::string>() >> 2)
        .when(__ >> 0)
        .end();
  }

  static int std_visit_route(const V &v) {
    return std::visit(
        [](const auto &x) -> int {
          using T = std::decay_t<decltype(x)>;
          if constexpr (std::is_same_v<T, int>) {
            return 1;
          }
          else if constexpr (std::is_same_v<T, std::string>) {
            return 2;
          }
          else {
            return 0;
          }
        },
        v);
  }

  static void BM_Patternia_Int(benchmark::State &state) {
    V v = 42;
    for (auto _ : state) {
      benchmark::DoNotOptimize(patternia_route(v));
    }
  }

  static void BM_Patternia_String(benchmark::State &state) {
    V v = std::string("abc");
    for (auto _ : state) {
      benchmark::DoNotOptimize(patternia_route(v));
    }
  }

  static void BM_StdVisit_Int(benchmark::State &state) {
    V v = 42;
    for (auto _ : state) {
      benchmark::DoNotOptimize(std_visit_route(v));
    }
  }

  static void BM_StdVisit_String(benchmark::State &state) {
    V v = std::string("abc");
    for (auto _ : state) {
      benchmark::DoNotOptimize(std_visit_route(v));
    }
  }
} // namespace

BENCHMARK(BM_Patternia_Int);
BENCHMARK(BM_Patternia_String);
BENCHMARK(BM_StdVisit_Int);
BENCHMARK(BM_StdVisit_String);
