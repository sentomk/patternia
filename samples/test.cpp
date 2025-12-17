#include "ptn/patternia.hpp"

using namespace ptn;

int main() {
  int  x = 42;
  auto r =
      match(x).when(lit(1) >> "one").when(lit(2) >> "two").when(__ >> "other");
}