#include <ptn/patternia.hpp>

struct Point {
  int x;
  int y;
};

int main() {
  static constexpr auto pt = ptn::has<&Point::x, &Point::y>;
  auto [x, y]              = pt;
  Point p{3, 4};
  // Automatic structured bindings are not visible to the
  // captureless lambda in PTN_ON, in C++17 or C++20.
  return ptn::match(p)
         | PTN_ON(ptn::$(pt)[x * x + y * y == 25] >> 0, ptn::_ >> 1);
}
