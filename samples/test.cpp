#include "ptn/patternia.hpp"
#include <iostream>

using namespace ptn;

int main() {
  int x = 42;
  match(x,
        cases(
            lit(42) >> [] { std::cout << "42"; },
            lit(20) >> [] { std::cout << "20"; },
            __ >> [] { std::cout << "Other"; }))
      .end();

  auto r = match(x,
                 cases(lit(42) >> "42",
                       lit(20) >> "20",
                       lit(1) >> "1",
                       __ >> "other"))
               .end();

  auto r1 = match(x)
                .when(lit(1) >> "1")
                .when(lit(42) >> "42")
                .when(__ >> "other")
                .end();
}