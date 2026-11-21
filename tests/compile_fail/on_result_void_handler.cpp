// EXPECT_ERROR: [Patternia.on]: a handler's result does not convert
// EXPECT_ERROR: to the result type given to on<R>(...)
#include <ptn/patternia.hpp>

int main() {
  using namespace ptn;
  int value = 1;
  // A void handler has no result to convert to a non-void R.
  return match(value) | on<int>(lit(1) >> 1, _ >> [] {});
}
