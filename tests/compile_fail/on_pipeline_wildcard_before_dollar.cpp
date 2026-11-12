// EXPECT_ERROR: case sequence contains unreachable cases
#include <ptn/patternia.hpp>

using namespace ptn;

int main() {
  int x = 1;
  return match(x) | on(_ >> 0, $ >> 1);
}
