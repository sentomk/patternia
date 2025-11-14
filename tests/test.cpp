#include "ptn/patternia.hpp"

using namespace ptn;

int main() {
  int  x   = 42;
  auto res = match(type_of(x))
                 .when(type_is<int> >> "int")
                 .when(type_is<char> >> "char")
                 .otherwise("unkown");
}