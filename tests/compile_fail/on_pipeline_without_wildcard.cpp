#include <ptn/patternia.hpp>

using namespace ptn;

int main() {
  int x = 1;
  auto r = match(x) |
           on{
               lit(1) >> 1,
               lit(2) >> 2,
           };
  (void)r;
  return 0;
}
