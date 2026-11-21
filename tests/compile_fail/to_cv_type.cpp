// EXPECT_ERROR: [Patternia.to]: to<T> returns a new value; drop
// EXPECT_ERROR: const or volatile from T
#include <ptn/patternia.hpp>

int main() {
  using namespace ptn;
  int value = 1;
  return match(value) | on($ >> to<const int>);
}
