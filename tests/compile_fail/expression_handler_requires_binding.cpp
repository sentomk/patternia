// EXPECT_ERROR: [Patternia.match] Handler signature does not match
// EXPECT_ERROR: the pattern's binding result.
#include <ptn/patternia.hpp>

using namespace ptn;

int main() {
  int n = 1;
  return match(n) | on(_ >> _ + 1);
}
