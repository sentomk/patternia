#include <ptn/patternia.hpp>

int main() {
  int x = 1;
  auto r = ptn::match(x)
               .when(ptn::__ >> 0)
               .when(ptn::lit(1) >> 1)
               .end();
  (void)r;
  return 0;
}
