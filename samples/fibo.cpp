#include "ptn/patternia.hpp"
#include <cstdint>

using namespace ptn;

std::uint64_t fib_rec(std::uint64_t n) {
  return match(n)
      .when(lit(0u) >> 0u)
      .when(lit(1u) >> 1u)
      .when(
          bind()[_ > 1u] >>
          [&](std::uint64_t v) { return fib_rec(v - 1) + fib_rec(v - 2); })
      .otherwise([] { return 0u; });
}