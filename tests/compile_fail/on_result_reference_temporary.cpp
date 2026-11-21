// EXPECT_ERROR: [Patternia.on]: a handler's result does not convert
// EXPECT_ERROR: A reference R must bind directly to a reference the
#include <ptn/patternia.hpp>

int main() {
  using namespace ptn;
  int value  = 1;
  int stored = 2;
  // `const long &` would bind to a temporary converted from the
  // handler's `int &`, which dangles once the match returns.
  const long &result = match(value)
                       | on<const long &>(
                           _ >> [&]() -> int & { return stored; });
  return static_cast<int>(result);
}
