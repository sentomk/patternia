// Codegen probe: eight runtime literals plus a wildcard, owned
// subject.
//
// `match(rvalue)` owns its subject. Runtime literals are compared
// in order, as for a borrowed subject, but the comparisons must be
// inlined: without that this match keeps the case pack in memory
// and calls out of line. See scripts/check_codegen.py.

#include <ptn/patternia.hpp>

extern "C" int ptn_codegen_probe(int x) {
  using namespace ptn;
  return match(int(x))
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
