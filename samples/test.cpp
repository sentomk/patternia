#include "ptn/patternia.hpp"
#include <iostream>
#include <variant>

using namespace ptn;

struct X {
  int x;
};

int main() {
  int x = 42;
  match(x,
        cases(
            lit(42) >> [] { std::cout << "42"; },
            lit(20) >> [] { std::cout << "20"; },
            ptn::__ >> [] { std::cout << "Other"; }))
      .end();

  auto r = match(x,
                 cases(lit(42) >> "42",
                       lit(20) >> "20",
                       lit(1) >> "1",
                       ptn::__ >> "other"))
               .end();

  auto r1 = match(x)
                .when(lit(1) >> "1")
                .when(lit(42) >> "42")
                .when(ptn::__ >> "other")
                .end();
  X xx;
  match(xx)
      .when(has<&X::x>() >> [] { std::cout << "xx"; })
      .when(__ >> [] { std::cout << "---"; })
      .end();

  using Point  = int;
  using Height = int;
  std::variant<Point, Height> v;

  Point p = 32;

  std::variant<int, char, std::string> pax;
  auto                                 vv = match(pax)
                .when(is<int>() >> "Point")
                .when(is<char>() >> "Height")
                .when(as<std::string>() >> "Str")
                .when(ptn::__ >> "Other")
                .end();

  auto vr = match(v)
                .when(alt<0>() >> "Point")
                .when(alt<1>() >> "Height")
                .when(ptn::__ >> "other")
                .end();

  std::cout << vr;
}