// EXPECT_ERROR: missing fallback
#include <ptn/patternia.hpp>

using namespace ptn;

int main() {
  int x = 1;
  return match(x) | on($[_ > 0] >> [](int v) { return v; });
}
