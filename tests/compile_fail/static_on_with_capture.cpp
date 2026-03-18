#include <ptn/patternia.hpp>

using namespace ptn;

int main() {
  int x    = 1;
  int base = 1;

  auto r = match(x)
           | static_on([base] {
               return on(
                   val<1> >> base,
                   __ >> 0
               );
             });
  (void)r;
  return 0;
}

