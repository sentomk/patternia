// EXPECT_ERROR: [Patternia.on]: expected a case created with
// FORBID_ERROR: binding_args
// FORBID_ERROR: missing fallback
// FORBID_ERROR: common_type
#include <ptn/patternia.hpp>

int main() {
  int value = 0;
  // Even without a fallback, case syntax is the first diagnostic.
  return ptn::match(value) | ptn::on("not a case");
}
