#include <ptn/patternia.hpp>

int main() {
  int x = 1;
  auto r = ptn::match(x, ptn::cases(
               ptn::bind()[ptn::_ > 0] >> 1,
               ptn::__ >> 0
           )).end();
  (void)r;
  return 0;
}
