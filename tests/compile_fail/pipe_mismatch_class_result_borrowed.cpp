// EXPECT_ERROR: [Patternia.pipe]: the callable after | cannot be
// EXPECT_ERROR: called with the result of the handler before it
// FORBID_ERROR: cannot convert
// FORBID_ERROR: ambiguous
#include <ptn/patternia.hpp>
#include <string>

int main() {
  using namespace ptn;
  // A borrowed subject. The match's result is a class with several
  // constructors, so the failed pipe must not be converted to it.
  int         value = 1;
  std::string s     = match(value)
                  | on(
                      $ >> [](int) { return 1; } |
                          [](const std::string &t) { return t; },
                      _ >> std::string("other"));
  return static_cast<int>(s.size());
}
