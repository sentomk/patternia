#include <ptn/patternia.hpp>

int main() {
  int x = 0;
  auto r = ptn::match(x)
               .when(ptn::__ >> 1)
               .otherwise(0);
  (void)r;
  return 0;
}
