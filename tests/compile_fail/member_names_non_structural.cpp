// EXPECT_ERROR: A member placeholder was used
// EXPECT_ERROR: in a guard on a non-structural pattern.
#include <ptn/patternia.hpp>

struct Point {
  int x;
};

int main() {
  auto [x]  = ptn::has<&Point::x>;
  int value = 3;
  // Tuple-like names have the same structural-only restriction as
  // the deprecated PTN_BIND names.
  return ptn::match(value)
         | ptn::on(ptn::$[x > 0] >> 1, ptn::_ >> 0);
}
