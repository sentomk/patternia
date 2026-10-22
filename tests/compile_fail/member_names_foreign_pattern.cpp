// EXPECT_ERROR: A member placeholder does not match any
// EXPECT_ERROR: member listed in this has<...>.
// FORBID_ERROR: [Patternia.guard]: placeholder is out of range
// FORBID_ERROR: tuple index must be in range
// FORBID_ERROR: tuple index out of bounds
#include <ptn/patternia.hpp>

struct Point {
  int x;
};

struct Other {
  int x;
};

int main() {
  auto [x] = ptn::has<&Point::x>;
  Other value{3};
  // An identically named member of another owner is not a match.
  return ptn::match(value)
         | ptn::on(ptn::$(ptn::has<&Other::x>)[x > 0] >> 1,
                   ptn::_ >> 0);
}
