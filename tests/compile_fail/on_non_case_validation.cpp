// EXPECT_ERROR: [Patternia.on]: expected a case created with
// FORBID_ERROR: binding_args
// FORBID_ERROR: common_type
#include <ptn/patternia.hpp>

int main() {
  ptn::core::common::static_assert_valid_case<int, int>();
}
