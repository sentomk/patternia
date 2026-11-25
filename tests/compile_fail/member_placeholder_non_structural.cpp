// EXPECT_ERROR: [Patternia.guard]: A member name in this guard
// FORBID_ERROR: Member names are bound only by $(has<...>).
#include <ptn/patternia.hpp>

struct Point {
  int x;
  int y;
};

PTN_BIND(Point, x, y);

int main() {
  int v = 3;
  // Member placeholders are only valid in guards attached to
  // has<...>. Using one on a non-structural pattern must fail.
  auto r = ptn::match(v) | ptn::on(ptn::$[x > 0] >> 1, ptn::_ >> 0);
  return r;
}
