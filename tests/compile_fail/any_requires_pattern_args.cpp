#include <ptn/patternia.hpp>

int main() {
  auto p = ptn::any(1, ptn::lit(2));
  (void)p;
  return 0;
}
