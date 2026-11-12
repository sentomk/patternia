// EXPECT_ERROR: A member placeholder does not match any
// FORBID_ERROR: tuple index out of bounds
#include <ptn/patternia.hpp>

using namespace ptn;

struct Point {
  int x;
  int y;
};

int main() {
  auto [x, y] = has<&Point::x, &Point::y>;
  Point p{1, 2};
  return match(p) | on($(has<&Point::x>) >> y + 1, _ >> 0);
}
