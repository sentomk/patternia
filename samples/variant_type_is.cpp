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
         | on(
             is<int>() >> "int",
             $(is<std::string>()) >>
                 [](const std::string &s) { return "str:" + s; },
             is<Point>($(has<&Point::x, &Point::y>)) >>
                 [](int x, int y) {
                   return "pt:" + std::to_string(x + y);
                 },
             _ >> [] { return std::string("other"); });
}

std::string
describe_as(const std::variant<int, std::string, Point> &v) {
  PTN_BIND(Point, x, y);

  // Same as describe(...), but uses the binding sugar
  // `$(is<T>())` and `$(has<...>)`.
  //
  // For destructuring, use `is<T>()` with `$(has<...>)` to
  // bind the extracted fields.
  //
  // Guards can be attached directly to binding patterns.
  return match(v)
         | on(
             is<int>() >> "int",
             $(is<std::string>())[_ != ""] >>
                 [](const std::string &s) { return "str:" + s; },
             is<Point>($(has<&Point::x, &Point::y>))[x > 0 && y > 0]
                 >>
                 [](int x, int y) {
                   return "pt:" + std::to_string(x + y);
                 },
             _ >> [] { return std::string("other"); });
}

int main() {
  std::variant<int, std::string, Point> v = 7;
  std::cout << describe(v) << "\n";

  v = std::string("");
  std::cout << describe_as(v) << "\n";

  v = Point{2, 3};
  std::cout << describe(v) << "\n";
}
