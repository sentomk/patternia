#include "ptn/patternia.hpp"
#include <string>
#include <iostream>

using namespace ptn;
using namespace ptn::patterns;

int main() {
  int  x   = 2;
  auto res = match(x)
                 .with(value(0), [](int) { return std::string{"zero"}; })
                 .with(value(42), [](int) { return std::string{"answer"}; })
                 .otherwise([](int) { return std::string{"other"}; });
  std::cout << res << '\n';
  return 0;
}
