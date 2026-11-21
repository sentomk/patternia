// EXPECT_ERROR: [Patternia.on]: on<R>(...) needs R to be void, an
// EXPECT_ERROR: object type or a reference
// FORBID_ERROR: does not convert to the result type
#include <ptn/patternia.hpp>

int main() {
  using namespace ptn;
  int value = 1;
  // A function type cannot be returned; a function pointer can.
  match(value) | on<int()>(_ >> 1);
}
