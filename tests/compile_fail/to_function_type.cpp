// EXPECT_ERROR: [Patternia.to]: to<T> needs T to be a complete,
// EXPECT_ERROR: non-array, non-abstract object type
#include <ptn/patternia.hpp>

int main() {
  using namespace ptn;
  int value = 1;
  match(value) | on($ >> to<int(int)>);
}
