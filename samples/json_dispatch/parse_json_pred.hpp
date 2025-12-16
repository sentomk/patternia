#pragma once

#include "nlohmann/json.hpp"

using json = nlohmann::json;

// --- type predicates ---

inline auto is_type(json::value_t t) {
  return [=](const json &v) { return v.type() == t; };
}

inline auto is_number = [](const json &v) { return v.is_number(); };

// --- structural predicates ---

inline auto is_empty_array = [](const json &v) {
  return v.is_array() && v.empty();
};

inline auto is_small_array = [](const json &v) {
  return v.is_array() && v.size() <= 3;
};

inline auto has_field(const std::string &key) {
  return [=](const json &v) { return v.is_object() && v.contains(key); };
}
