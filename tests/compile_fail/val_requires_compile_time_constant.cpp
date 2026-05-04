#include "ptn/patternia.hpp"

// `argc` is a truly runtime value — non-type template parameter must
// be a converted constant expression (C++17 [temp.arg.nontype]/1).
// The compiler will reject this at template argument deduction.
int main(int argc, char **) {
  auto pattern = ptn::val<argc>; // expected-error: non-type template
                                 // argument is not a constant
                                 // expression
  (void) pattern;
  return 0;
}
