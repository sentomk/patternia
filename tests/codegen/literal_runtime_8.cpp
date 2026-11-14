// Codegen probe: eight runtime literals plus a wildcard.
//
// `lit(...)` values are compared in order. The case pack must still
// stay out of memory. See scripts/check_codegen.py.

#include <ptn/patternia.hpp>

extern "C" int ptn_codegen_probe(int x) {
  using namespace ptn;
  return match(x)
         | on(lit(1) >> 11,
              lit(2) >> 22,
              lit(3) >> 33,
              lit(4) >> 44,
              lit(5) >> 55,
              lit(6) >> 66,
              lit(7) >> 77,
              lit(8) >> 88,
              _ >> 0);
}
