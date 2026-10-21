// EXPECT_ERROR: A member placeholder does not match any
// EXPECT_ERROR: member listed in this has<...>.
// FORBID_ERROR: [Patternia.guard]: placeholder is out of range
// FORBID_ERROR: tuple index must be in range
// FORBID_ERROR: tuple index out of bounds
#include <ptn/patternia.hpp>

struct Point {
  int x;
  int y;
};

int main() {
  auto [x, y] = ptn::has<&Point::x, &Point::y>;
  Point value{3, 4};
  // Ignored slots do not make y available to a nested expression.
  return ptn::match(value)
         | ptn::on(
             ptn::$(ptn::has<ptn::_ign, &Point::x, ptn::_ign>)[x + y
                                                               > 0]
                 >> 1,
             ptn::_ >> 0);
}
