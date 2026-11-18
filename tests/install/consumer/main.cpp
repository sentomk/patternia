// Compiled against an installed copy of Patternia, not the source
// tree.
#include <ptn/patternia.hpp>

#include <memory>
#include <string>
#include <variant>

int main() {
  using namespace ptn;

  int x     = 3;
  int dense = match(x)
              | on(val<1> >> 1, val<2> >> 2, val<3> >> 3, _ >> 0);

  std::variant<int, std::string> v   = std::string("ok");
  int                            alt = match(v)
            | on(is<int>() >> 0, is<std::string>() >> 1, _ >> 2);

  int owned = match(std::make_unique<int>(7)) | on($ >> 1);

  return dense == 3 && alt == 1 && owned == 1 ? 0 : 1;
}
