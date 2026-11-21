// EXPECT_ERROR: [Patternia.on]: on<R>(...) needs R to be void, an
// EXPECT_ERROR: object type or a reference
// FORBID_ERROR: does not convert to the result type
#include <ptn/patternia.hpp>

int main() {
  using namespace ptn;
  int value = 1;
  match(value) | on<int[2]>(_ >> 1);
}
