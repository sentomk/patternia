// EXPECT_ERROR: [Patternia.to]: T cannot be constructed from the
// FORBID_ERROR: handler cannot accept the owned
// FORBID_ERROR: no matching
#include <ptn/patternia.hpp>
#include <string>

int main() {
  using namespace ptn;
  // An owned subject reports the same diagnostic as a borrowed one.
  std::string s = match(1)
                  | on($ >> to<std::string>, _ >> std::string("x"));
  return static_cast<int>(s.size());
}
