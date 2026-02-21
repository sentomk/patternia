#include <ptn/patternia.hpp>

using namespace ptn;

int main() {
  int x = 1;
  auto r = (match(x).when(lit(1) >> 1) |
            on{
                __ >> 0,
            })
               .end();
  (void)r;
  return 0;
}
