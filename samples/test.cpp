#include "ptn/patternia.hpp"
#include <iostream>

using namespace ptn;

int main() {
  int x = 42;
  match(
      x,
      cases(
          lit(42) >> [] { std::cout << "42"; },
          lit(20) >> [] { std::cout << "20"; },
          __ >> [] { std::cout << "Other"; }))
      .end();
}