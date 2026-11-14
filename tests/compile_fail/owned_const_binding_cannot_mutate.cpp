// EXPECT_ERROR: [Patternia.match]: handler cannot accept the owned
#include <ptn/patternia.hpp>
#include <utility>
int main() {
  using namespace ptn;
  const int value = 1;
  match(std::move(value)) | on($ >> [](int &&v) { ++v; });
}
