// EXPECT_ERROR: Handler signature does not match the pattern's
// FORBID_ERROR: does not convert to the result type
#include <ptn/patternia.hpp>
#include <string>

int main() {
  using namespace ptn;
  int value = 1;
  // The binding is an int; the handler wants a string.
  return match(value)
         | on<int>($ >> [](const std::string &) { return 1; });
}
