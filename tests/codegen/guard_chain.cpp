// Codegen probe: guarded bindings that fall through to `$`.
//
// A chain of guards on a scalar must compile to plain comparisons.
// See scripts/check_codegen.py.

#include <ptn/patternia.hpp>

extern "C" int ptn_codegen_probe(int x) {
  using namespace ptn;
  return match(x)
         | on($[_ > 1000] >> _ + 4,
              $[_ > 100] >> _ + 3,
              $[_ > 10] >> _ + 2,
              $[_ > 1] >> _ + 1,
              $ >> _ * 2);
}
