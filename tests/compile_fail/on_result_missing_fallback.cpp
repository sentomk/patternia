// EXPECT_ERROR: [Patternia.on]: missing fallback
// FORBID_ERROR: does not convert to the result type
#include <ptn/patternia.hpp>

int main() {
  using namespace ptn;
  int value = 1;
  // on<R> fixes the result type only; it does not supply a fallback.
  return match(value) | on<int>(lit(1) >> 1);
}
