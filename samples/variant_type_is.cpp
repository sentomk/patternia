#include <iostream>
#include <string>
#include <variant>

#include "ptn/patternia.hpp"

using namespace ptn;

struct Point {
  int x;
  int y;
};

std::string describe(const std::variant<int, std::string, Point> &v) {
  return match(v)
      .when(type::is<int>() >> [] { return std::string("int"); })
      .when(
          type::is<std::string>(bind()) >>
          [](const std::string &s) { return "str:" + s; }
      )
      .when(
          type::is<Point>(bind(has<&Point::x, &Point::y>())) >>
          [](int x, int y) { return "pt:" + std::to_string(x + y); }
      )
      .otherwise([] { return std::string("other"); });
}

int main() {
  std::variant<int, std::string, Point> v = 7;
  std::cout << describe(v) << "\n";

  v = std::string("hi");
  std::cout << describe(v) << "\n";

  v = Point{2, 3};
  std::cout << describe(v) << "\n";
}
