// EXPECT_ERROR: [Patternia.guard]: A member name in this guard
// FORBID_ERROR: Member names are bound only by $(has<...>).
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
