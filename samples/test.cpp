#include <iostream>
#include <string>
#include "ptn/patternia.hpp"

using namespace ptn;

auto is(int v) {
  return lit(v);
}

int main() {

  int x = 2;

  std::cout << "---- Test 1: expression match (returns string) ----\n";

  auto r1 = match(x)
                .when(lit(1) >> [](int) { return std::string("one"); })
                .when(lit(2) >> [](int) { return std::string("two"); })
                .otherwise([](int) { return std::string("other"); });

  std::cout << "r1 = " << r1 << "\n\n";

  std::cout << "---- Test 2: statement match (all case return void) ----\n";

  match(x)
      .when(lit(1) >> [](int) { std::cout << "case 1\n"; })
      .when(lit(2) >> [](int) { std::cout << "case 2\n"; })
      .otherwise([](int) { std::cout << "otherwise\n"; });

  std::cout << "\n";

  std::cout
      << "---- Test 3: mismatch return types (should fail to compile) ----\n";
  std::cout << "(comment this block out to continue compilation)\n";

  /*
  auto r3 = match(x)
      .when(is(1) >> [](int){ return 10; })            // int
      .when(is(2) >> [](int){ return std::string("a"); }) // std::string
      .otherwise([](int){ return 3.0; });              // double
  */

  return 0;
}
