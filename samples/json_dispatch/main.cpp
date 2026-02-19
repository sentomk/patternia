#include "nlohmann/json.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

using json = nlohmann::json;

// Forward declaration.
void parse_json(const json &j, int depth = 0);

static json load_json_file(const std::string &path) {
  std::ifstream ifs(path);
  if (!ifs) {
    throw std::runtime_error("failed to open json file: " + path);
  }
  return json::parse(ifs);
}

int main(int argc, char **argv) {

  if (argc < 2) {
    std::cerr << "usage: json_parser <file.json>\n";
    return 1;
  }

  try {
    json root = load_json_file(argv[1]);
    parse_json(root);
  } catch (const json::parse_error &e) {
    std::cerr << "json parse error: " << e.what() << "\n";
    return 2;
  } catch (const std::exception &e) {
    std::cerr << "error: " << e.what() << "\n";
    return 3;
  }

  return 0;
}
