// EXPECT_ERROR: [Patternia.handler]: A member name in this
// FORBID_ERROR: Handler signature does not match
#include <ptn/patternia.hpp>

using namespace ptn;

struct Point {
  int x;
  int y;
};

int main() {
  auto [x, y] = has<&Point::x, &Point::y>;
  int n       = 1;
  return match(n) | on($ >> x + 1);
}
