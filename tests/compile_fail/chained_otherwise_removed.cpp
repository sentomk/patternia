#include <ptn/patternia.hpp>

int main() {
  int x = 1;
  auto r = ptn::match(x).otherwise(0);
  (void)r;
  return 0;
}
