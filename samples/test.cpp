#include "ptn/patternia.hpp"
#include <iostream>
#include <variant>
#include <string>

using namespace ptn;
using namespace ptn::pat::value;
using namespace ptn::pat::type;

void basic_examples() {
  // 1. Simple value matching
  int  x       = 42;
  auto result1 = match(x)
                     .when(lit(42) >> "answer to everything")
                     .when(lit(0) >> "zero")
                     .otherwise("other number");

  // 2. Relational patterns
  double score   = 85.5;
  auto   result2 = match(score)
                     .when(ge(90) >> 'A')
                     .when(ge(80) >> 'B')
                     .when(ge(70) >> 'C')
                     .when(ge(60) >> 'D')
                     .otherwise('F');

  std::cout << "Result 1: " << result1 << '\n';
  std::cout << "Result 2: " << result2 << '\n';
}