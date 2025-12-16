#include "ptn/patternia.hpp"
#include "parse_json_pred.hpp"
#include "parse_json_handler.hpp"

using namespace ptn;

void parse_json(const json &j, int depth = 0) {

  match(j)

      .when(bind()[is_type(json::value_t::null)] >> print_null(depth))

      .when(bind()[is_type(json::value_t::boolean)] >> print_bool(depth))

      .when(
          bind()[is_type(json::value_t::number_integer)] >>
          print_number<int>("int", depth))

      .when(
          bind()[is_type(json::value_t::number_unsigned)] >>
          print_number<unsigned>("uint", depth))

      .when(
          bind()[is_type(json::value_t::number_float)] >>
          print_number<double>("float", depth))

      .when(bind()[is_type(json::value_t::string)] >> print_string(depth))

      .when(
          bind()[is_empty_array] >>
          [=](const json &) {
            indent(depth);
            std::cout << "array []\n";
          })

      .when(
          bind()[is_type(json::value_t::array)] >>
          [=](const json &arr) {
            indent(depth);
            std::cout << "array (" << arr.size() << ")\n";
            for (auto &e : arr) {
              parse_json(e, depth + 1);
            }
          })

      .when(
          bind()[has_field("name")] >>
          [=](const json &obj) {
            indent(depth);
            std::cout << "object <named>\n";
            for (auto &&[k, v] : obj.items()) {
              indent(depth + 1);
              std::cout << k << ":\n";
              parse_json(v, depth + 2);
            }
          })

      .otherwise([=] {
        indent(depth);
        std::cout << "<unknown>\n";
      });
}
