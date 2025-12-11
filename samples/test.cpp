// Simple test program demonstrating Patternia pattern matching.
//
// This program shows basic usage of the Patternia library including
// literal patterns, wildcard patterns, and the match DSL.

#include <iostream>
#include "ptn/patternia.hpp"

using namespace ptn;

int main() {
  int x = 114;

  match(x)
      .when(bind()[_ < 20] >> [](int v) { std::cout << v << "Bigger than 20"; })
      .when(lit(42) >> [] { std::cout << "42"; })
      .when(bind(__)[_ > 100] >> [](int v) { std::cout << v << " Other"; })
      .end();
}