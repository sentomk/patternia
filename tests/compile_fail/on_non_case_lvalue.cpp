// EXPECT_ERROR: [Patternia.on]: expected a case created with
// EXPECT_ERROR: built-in bit shift
// FORBID_ERROR: binding_args
// FORBID_ERROR: common_type
#include <ptn/patternia.hpp>

int main() {
  using namespace ptn;
  int  value = 0;
  auto cases = on(_ >> 0, 0 >> 1);
  return match(value) | cases;
}
