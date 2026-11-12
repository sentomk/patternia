// EXPECT_ERROR: [Patternia.handler]: placeholder is out of range
// FORBID_ERROR: tuple index out of bounds
#include <ptn/patternia.hpp>

using namespace ptn;

int main() {
  int n = 1;
  return match(n) | on($ >> pat::mod::arg_t<1>{} + 1);
}
