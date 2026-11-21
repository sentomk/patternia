// EXPECT_ERROR: [Patternia.pipe]: the callable after | cannot be
// EXPECT_ERROR: called with the result of the handler before it
// FORBID_ERROR: Handler signature does not match
#include <ptn/patternia.hpp>
#include <string>

int main() {
  using namespace ptn;
  int value = 1;
  // The handler yields an int; the callable wants a string. Checked
  // even though the case is never selected.
  return match(value)
         | on(
             $[_ == 0] >> _ |
                 [](const std::string &s) {
                   return static_cast<int>(s.size());
                 },
             _ >> 1);
}
