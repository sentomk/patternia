#include <iostream>
#include <string>
#include <variant>

#include "ptn/patternia.hpp"

using namespace ptn;

struct Point {
  int x;
  int y;
};

std::string
describe(const std::variant<int, std::string, Point> &v) {
  return match(v)
      .when(is<int>() >> "int")
      .when(is<std::string>(bind()) >>
            [](const std::string &s) { return "str:" + s; })
      .when(
          is<Point>(bind(has<&Point::x, &Point::y>())) >>
          [](int x, int y) { return "pt:" + std::to_string(x + y); })
      .otherwise([] { return std::string("other"); });
}

std::string
describe_as(const std::variant<int, std::string, Point> &v) {
  // Same as describe(...), but uses the binding sugar
  // `$(is<T>())` and `$(has<...>())`.
  //
  // `$(is<T>())` is equivalent to `is<T>(bind())`, so the
  // handler receives the alternative value.
  //
  // For destructuring, use `is<T>()` with `$(has<...>())` to
  // bind the extracted fields.
  //
  // Guards can be attached directly to binding patterns.
  return match(v)
      .when(is<int>() >> "int")
      .when($(is<std::string>())[_0 != ""] >>
            [](const std::string &s) { return "str:" + s; })
      .when(
          is<Point>($(
              has<&Point::x, &Point::y>()))[arg<0> > 0 && arg<1> > 0]
          >>
          [](int x, int y) { return "pt:" + std::to_string(x + y); })
      .otherwise([] { return std::string("other"); });
}

int main() {
  std::variant<int, std::string, Point> v = 7;
  std::cout << describe(v) << "\n";

  v = std::string("");
  std::cout << describe_as(v) << "\n";

  v = Point{2, 3};
  std::cout << describe(v) << "\n";
}
