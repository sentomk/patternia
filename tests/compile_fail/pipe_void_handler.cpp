// EXPECT_ERROR: [Patternia.pipe]: the handler before | returns void
// FORBID_ERROR: cannot be called with the result
// FORBID_ERROR: Handler signature does not match
#include <ptn/patternia.hpp>

int main() {
  using namespace ptn;
  int value = 1;
  match(value) | on(_ >> [] {} | [](int) {});
}
