// EXPECT_ERROR: [Patternia.on]: a handler's result does not convert
// EXPECT_ERROR: to the result type given to on<R>(...)
// FORBID_ERROR: common_type
#include <ptn/patternia.hpp>
#include <memory>

int main() {
  using namespace ptn;
  // Owned subject: results are checked against the owned bindings.
  auto result = match(std::make_unique<int>(1))
                | on<int>($ >>
                          [](std::unique_ptr<int> p) { return p; });
  return result;
}
