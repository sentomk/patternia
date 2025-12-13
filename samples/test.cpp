// Simple test program demonstrating Patternia pattern matching.
//
// This program shows basic usage of the Patternia library including
// literal patterns, wildcard patterns, and the match DSL.
struct Point {
  int x;
  int y;
  int z;
};

#include <iostream>
#include "ptn/patternia.hpp"

using namespace ptn;

enum class Status {
  Walking,
  Running,
  IDLE
};

void example_structural(const Point &p) {
  match(p)
      .when(
          bind(has<&Point::x, &Point::y, nullptr>()) >>
          [](int x, int y) { std::cout << x << ' ' << y; })
      .end();
}

int main() {
  int x = 1554;

  match(x)
      .when(bind()[_ == 0] >> [](int v) { std::cout << v << "Bigger than 20"; })
      .when(lit(42) >> [] { std::cout << "42"; })
      .when(
          bind(__)[_ > 100 && _ < 200] >>
          [](int v) { std::cout << v << " Other"; })
      .otherwise([] { std::cout << "Nothing matched\n"; });

  Status status = Status::IDLE;
  match(status)
      .when(lit(Status::Walking) >> [] { std::cout << "Walking"; })
      .when(
          bind(__) >>
          [](Status v) { std::cout << static_cast<int>(v) << '\n'; })
      .end();

  Point point{1, 2, 3};
  example_structural(point);
}
