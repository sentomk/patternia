#include "ptn/core/dsl/ops.hpp"
#include "ptn/core/engine/match.hpp"
#include "ptn/patternia.hpp"
#include <iostream>
#include <variant>
#include <charconv>
#include <string>
#include <string_view>

enum Direction {
  North,
  South,
  East,
  West,
};

int main() {
  using namespace ptn;

  using V = std::variant<int, std::string, char>;

  V v1;

  std::string line;
  std::getline(std::cin, line);

  if (line.size() == 1 && !std::isdigit(static_cast<unsigned char>(line[0]))) {
    v1 = line[0];
  }
  else {
    int  value     = 0;
    auto sv        = std::string_view(line);
    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);
    if (ec == std::errc{} && ptr == sv.data() + sv.size()) {
      v1 = value;
    }
    else {
      v1 = line;
    }
  }

  auto res = match(v1)
             | on{is<std::string>() >> "String",
                  is<int>() >> "Integer",
                  is<char>() >> "Character",
                  __ >> "Other"};

  using X = int;
  using Y = int;
  using Z = int;

  using Point = std::variant<X, Y, Z>;

  Point p{std::in_place_index<0>, 42};

  match(p)
      | on{alt<0>(bind())[_ > 0] >>
               [](auto v) { std::cout << "X: " << v << '\n'; },
           alt<1>(bind()) >> [](auto v) { std::cout << "Y: " << v << '\n'; },
           alt<2>(bind()) >> [](auto v) { std::cout << "Z: " << v << '\n'; },
           __ >> [] { std::cout << "Other\n"; }};

  std::cout << res << '\n';
}