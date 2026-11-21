// EXPECT_ERROR: [Patternia.to]: to<T> needs T to be a complete,
// EXPECT_ERROR: non-array, non-abstract object type
#include <ptn/patternia.hpp>

struct Shape {
  virtual ~Shape()          = default;
  virtual int sides() const = 0;
};

int main() {
  using namespace ptn;
  int value = 1;
  match(value) | on(_ >> to<Shape>);
}
