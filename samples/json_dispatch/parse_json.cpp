#include "ptn/patternia.hpp"
#include "parse_json_pred.hpp"
#include "parse_json_handler.hpp"

using namespace ptn;

void parse_json(const json &j, int depth = 0) {

  match(j)
      | on(
          $[is_type(json::value_t::null)] >> print_null(depth),
          $[is_type(json::value_t::boolean)] >> print_bool(depth),
          $[is_type(json::value_t::number_integer)] >>
              print_number<int>("int", depth),
          $[is_type(json::value_t::number_unsigned)] >>
              print_number<unsigned>("uint", depth),
          $[is_type(json::value_t::number_float)] >>
              print_number<double>("float", depth),
          $[is_type(json::value_t::string)] >> print_string(depth),
          $[is_empty_array] >>
              [=](const json &) {
                indent(depth);
                std::cout << "array []\n";
              },
          $[is_type(json::value_t::array)] >>
              [=](const json &arr) {
                indent(depth);
                std::cout << "array (" << arr.size() << ")\n";
                for (auto &e : arr) {
                  parse_json(e, depth + 1);
                }
              },
          $[has_field("name")] >>
              [=](const json &obj) {
                indent(depth);
                std::cout << "object <named>\n";
                for (auto &&[k, v] : obj.items()) {
                  indent(depth + 1);
                  std::cout << k << ":\n";
                  parse_json(v, depth + 2);
                }
              },
          __ >> [=] {
            indent(depth);
            std::cout << "<unknown>\n";
          });
}
