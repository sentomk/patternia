// EXPECT_ERROR: [Patternia.to]: T cannot be constructed from the
// FORBID_ERROR: Handler signature does not match
// FORBID_ERROR: no matching
#include <ptn/patternia.hpp>
#include <string>

struct Point {
  int x;
  int y;
};

int main() {
  using namespace ptn;
  int value = 1;
  // to<Point> after | receives the handler's std::string result.
  Point p = match(value)
            | on(
                $ >> [](int) { return std::string("a"); }
                    | to<Point>,
                _ >> Point{});
  return p.x;
}
