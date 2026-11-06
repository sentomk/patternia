// EXPECT_ERROR: [Patternia.all]: every argument must be a pattern
#include <ptn/patternia.hpp>

int main() {
  // A callable is neither a pattern nor a plain value.
  auto p = ptn::all([](int) { return true; }, ptn::lit(2));
  (void) p;
  return 0;
}
