// EXPECT_ERROR: [Patternia.match]: handler cannot accept the owned
#include <ptn/patternia.hpp>
struct Handler {
  void operator()(int) && {
  }
};
int main() {
  using namespace ptn;
  auto cases = on($ >> Handler{});
  match(1) | cases;
}
