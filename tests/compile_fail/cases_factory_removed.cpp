#include <ptn/patternia.hpp>

int main() {
  auto pack = ptn::cases(ptn::lit(1) >> 1, ptn::__ >> 0);
  (void)pack;
  return 0;
}
