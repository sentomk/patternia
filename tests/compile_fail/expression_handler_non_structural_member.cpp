// EXPECT_ERROR: A member placeholder was used
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
