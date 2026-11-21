// EXPECT_ERROR: [Patternia.on]: on<R>(...) cannot return an
// EXPECT_ERROR: abstract class by value
// FORBID_ERROR: does not convert to the result type
#include <ptn/patternia.hpp>

struct Shape {
  virtual ~Shape()          = default;
  virtual int sides() const = 0;
};

struct Square : Shape {
  int sides() const override {
    return 4;
  }
};

int main() {
  using namespace ptn;
  int    value = 1;
  Square square;
  // An abstract class can only be returned by reference or pointer.
  match(value) | on<Shape>(_ >> [&]() -> Shape & { return square; });
}
