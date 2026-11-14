// EXPECT_ERROR: [Patternia.match]: handler cannot accept the owned
#include <ptn/patternia.hpp>
#include <memory>
int main() {
  using namespace ptn;
  auto cases = on($ >> std::make_unique<int>(1));
  match(1) | cases;
}
