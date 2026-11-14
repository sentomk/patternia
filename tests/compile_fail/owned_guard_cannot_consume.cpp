// EXPECT_ERROR: [Patternia.guard]: guards on owned subjects must
// EXPECT_ERROR: accept read-only bindings
#include <ptn/patternia.hpp>
#include <memory>
int main() {
  using namespace ptn;
  return match(std::make_unique<int>(1))
         | on($[([](std::unique_ptr<int>) { return true; })] >> 1,
              _ >> 0);
}
