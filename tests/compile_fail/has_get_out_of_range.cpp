// EXPECT_ERROR: [Patternia.has]: member index is out of range.
// FORBID_ERROR: incomplete type
// FORBID_ERROR: undefined template
#include <ptn/patternia.hpp>

struct Point {
  int x;
};

int main() {
  (void) ptn::pat::detail::get<1>(ptn::has<&Point::x>);
}
