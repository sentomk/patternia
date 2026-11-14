// Codegen probe: 16 static literals plus a wildcard.
//
// Dense literal dispatch must reduce to a range check and a table
// or arithmetic. See scripts/check_codegen.py.

#include <ptn/patternia.hpp>

#define PROBE_CASE(n) ptn::val<n> >> ((n) * 3)
#define PROBE_BLOCK_4(b)                                            \
  PROBE_CASE((b) + 0), PROBE_CASE((b) + 1), PROBE_CASE((b) + 2),    \
      PROBE_CASE((b) + 3)
#define PROBE_BLOCK_16(b)                                           \
  PROBE_BLOCK_4((b) + 0), PROBE_BLOCK_4((b) + 4),                   \
      PROBE_BLOCK_4((b) + 8), PROBE_BLOCK_4((b) + 12)

extern "C" int ptn_codegen_probe(int x) {
  using namespace ptn;
  return match(x) | on(PROBE_BLOCK_16(1), _ >> 0);
}
