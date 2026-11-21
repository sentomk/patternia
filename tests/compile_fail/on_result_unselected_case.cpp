// EXPECT_ERROR: [Patternia.on]: a handler's result does not convert
// EXPECT_ERROR: to the result type given to on<R>(...)
// FORBID_ERROR: common_type
#include <ptn/patternia.hpp>
#include <string>

int main() {
  using namespace ptn;
  int value = 1;
  // The string case is never selected for this subject, but every
  // handler result must convert to R.
  return match(value)
         | on<int>(lit(1) >> 1, _ >> std::string("other"));
}
