// EXPECT_ERROR: [Patternia.match]: handlers must all return void or
// EXPECT_ERROR: values with a common result type
#include <ptn/patternia.hpp>
#include <memory>
#include <string>
int main() {
  using namespace ptn;
  match(1)
      | on(lit(1) >> std::make_unique<int>(1), _ >> std::string{});
}
