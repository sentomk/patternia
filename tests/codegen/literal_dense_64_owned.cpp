// Codegen probe: 64 static literals plus a wildcard, owned subject.
//
// `match(rvalue)` owns its subject and normally evaluates the cases
// in source order. A dense run of static literals must still use
// keyed dispatch: without it this match compares case by case and
// writes all 64 cases to the stack on every call.
// See scripts/check_codegen.py.

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
  return match(int(x))
         | on(PROBE_BLOCK_16(1),
              PROBE_BLOCK_16(17),
              PROBE_BLOCK_16(33),
              PROBE_BLOCK_16(49),
              _ >> 0);
}
