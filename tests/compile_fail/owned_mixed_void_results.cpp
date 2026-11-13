// EXPECT_ERROR: [Patternia.match]: handlers must all return void or
// EXPECT_ERROR: values with a common result type
#include <ptn/patternia.hpp>
int main() {
  using namespace ptn;
  match(1) | on(lit(1) >> [] {}, _ >> 0);
}
