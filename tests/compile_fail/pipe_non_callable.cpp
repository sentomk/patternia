// EXPECT_ERROR: [Patternia.pipe]: the right side of | must be
// callable
#include <ptn/patternia.hpp>

int main() {
  using namespace ptn;
  int value = 1;
  return match(value) | on($ >> _ | 42);
}
