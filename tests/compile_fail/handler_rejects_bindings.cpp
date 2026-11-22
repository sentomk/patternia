// EXPECT_ERROR: Handler signature does not match the pattern's
// FORBID_ERROR: At least one case's handler
#include <ptn/patternia.hpp>
#include <string>

int main() {
  using namespace ptn;
  int value = 1;
  // The handler accepts neither the int binding nor no arguments.
  return match(value)
         | on($ >> [](const std::string &) { return 1; }, _ >> 0);
}
