// EXPECT_ERROR: [Patternia.on]: expected a case created with
// EXPECT_ERROR: built-in bit shift
// EXPECT_ERROR: lit(0) >> 1
// FORBID_ERROR: binding_args
#include <ptn/patternia.hpp>

int main() {
  using namespace ptn;
  // Reject the malformed pack even before it is matched.
  auto cases = on(0 >> 1, _ >> 0);
  (void) cases;
}
