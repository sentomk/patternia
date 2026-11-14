// EXPECT_ERROR: [Patternia.match] Handler signature does not
#include <ptn/patternia.hpp>
#include <memory>
int main() {
  using namespace ptn;
  auto value = std::make_unique<int>(1);
  match(value) | on($ >> [](std::unique_ptr<int>) {});
}
