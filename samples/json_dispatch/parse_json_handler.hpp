#pragma once

#include "nlohmann/json.hpp"
#include <iostream>

using json = nlohmann::json;

inline void indent(int depth) {
  while (depth--)
    std::cout << "  ";
}

// --- scalar handlers ---

inline auto print_null(int depth) {
  return [=](auto &&...) {
    indent(depth);
    std::cout << "null\n";
  };
}

inline auto print_bool(int depth) {
  return [=](const json &v) {
    indent(depth);
    std::cout << "bool: " << std::boolalpha << v.get<bool>() << "\n";
  };
}

template <typename T>
inline auto print_number(const char *label, int depth) {
  return [=](const json &v) {
    indent(depth);
    std::cout << label << ": " << v.get<T>() << "\n";
  };
}

inline auto print_string(int depth) {
  return [=](const json &v) {
    indent(depth);
    std::cout << "string: \"" << v.get<std::string>() << "\"\n";
  };
}
