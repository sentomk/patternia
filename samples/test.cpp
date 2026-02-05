#include "ptn/patternia.hpp"
#include <iostream>
#include <utility>
#include <variant>

using namespace ptn;

int main() {

  using Point  = int;
  using Height = int;

  using V = std::variant<Point, Height>;

  V v{std::in_place_index<1>, 23};

  auto res = match(v)
                 .when(alt<0>() >> "Point")
                 .when(alt<1>() >> "Height")
                 .when(ptn::__ >> "Other")
                 .end();

  std::cout << res;
}