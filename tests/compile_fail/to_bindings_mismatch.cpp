// EXPECT_ERROR: [Patternia.to]: T cannot be constructed from the
// FORBID_ERROR: Handler signature does not match
// FORBID_ERROR: no matching
#include <ptn/patternia.hpp>
#include <string>

int main() {
  using namespace ptn;
  int value = 1;
  // std::string has a default constructor, but to<T> is not called
  // without the int binding.
  std::string s = match(value)
                  | on($ >> to<std::string>, _ >> std::string("x"));
  return static_cast<int>(s.size());
}
