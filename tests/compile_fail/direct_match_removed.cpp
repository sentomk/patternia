#include <ptn/patternia.hpp>

int main() {
  int x = 1;
  auto r = ptn::match(x, ptn::lit(1) >> 1, ptn::__ >> 0);
  (void)r;
  return 0;
}
