// EXPECT_ERROR: [Patternia.on]: expected a case created with
// EXPECT_ERROR: built-in bit shift
// EXPECT_ERROR: lit(0) >> 1
// FORBID_ERROR: binding_args
// FORBID_ERROR: missing fallback
// FORBID_ERROR: common_type
#include <ptn/patternia.hpp>

int main() {
  using namespace ptn;
  int value = 0;
  return match(value) | on(0 >> 1, _ >> 0);
}
