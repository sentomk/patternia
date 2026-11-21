// EXPECT_ERROR: [Patternia.pipe]: the right side of | is a pattern
// FORBID_ERROR: must be callable
#include <ptn/patternia.hpp>

int main() {
  using namespace ptn;
  int value = 1;
  // A pattern on the right of | would be called with the result.
  return match(value) | on($ >> _ | is<int>);
}
