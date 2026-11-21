// EXPECT_ERROR: [Patternia.on]: a handler's result does not convert
// EXPECT_ERROR: to the result type given to on<R>(...)
#include <ptn/patternia.hpp>

struct Wrapper {
  explicit Wrapper(int v) : value(v) {
  }
  int value;
};

int main() {
  using namespace ptn;
  int value = 1;
  // An explicit constructor is not an implicit conversion.
  Wrapper w = match(value) | on<Wrapper>(_ >> 1);
  return w.value;
}
