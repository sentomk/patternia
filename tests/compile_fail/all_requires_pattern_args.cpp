#include <ptn/patternia.hpp>

int main() {
  auto p = ptn::all(1, ptn::lit(2));
  (void)p;
  return 0;
}
