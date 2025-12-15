#include "ptn/patternia.hpp"
#include <cstdint>
#include <iostream>

using namespace ptn;

std::uint64_t fib(std::uint64_t n) {

  // is_even predicate
  auto is_even = [](auto v) { return v % 2u == 0u; };

  // is_odd predicate
  auto is_odd = [](auto v) { return v % 2u == 1u; };

  return match(n)
      .when(lit(0u) >> 0u)
      .when(lit(1u) >> 1u)

      // even n > 1
      .when(
          bind()[_ > 1u && is_even] >>
          [&](std::uint64_t v) { return fib(v - 1) + fib(v - 2); })

      // odd n > 1
      .when(
          bind()[_ > 1u && is_odd] >>
          [&](std::uint64_t v) { return fib(v - 1) + fib(v - 2); })
      .otherwise([] { return 0u; });
}

int main() {
  for (std::uint64_t i = 0; i <= 10; ++i) {
    std::cout << "fib(" << i << ") = " << fib(i) << "\n";
  }
}