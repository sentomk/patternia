#include <ptn/patternia.hpp>

int main() {
  int x = 1;
  auto r = ptn::match(x).end();
  (void)r;
  return 0;
}
