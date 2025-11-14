#include "ptn/patternia.hpp"
#include "ptn/type/type_tag.hpp"
#include <vector>

using namespace ptn;

int main() {
  std::vector<int> v;
  auto out = match(v).when(type::from<std::vector> >> "vector").otherwise("no");
}