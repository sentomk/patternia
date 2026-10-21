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

PTN_BIND(Point, x, y);

int main() {
  Point p{3, 4};
  // has<...> does not list &Point::y, so the member placeholder
  // y cannot be resolved: static_assert must fire.
  auto r = ptn::match(p)
           | ptn::on(ptn::$(ptn::has<&Point::x>)[y > 0] >> 1,
                     ptn::_ >> 0);
  return r;
}
