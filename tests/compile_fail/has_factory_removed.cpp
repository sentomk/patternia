#include <ptn/patternia.hpp>

struct Point {
  int x;
};

int main() {
  auto p = ptn::has<&Point::x>();
  (void)p;
  return 0;
}
