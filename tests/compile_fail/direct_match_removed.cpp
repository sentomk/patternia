#include <ptn/patternia.hpp>

int main() {
  int  x = 1;
  auto r = ptn::match(x, ptn::lit(1) >> 1, ptn::_ >> 0);
  (void) r;
  return 0;
}
