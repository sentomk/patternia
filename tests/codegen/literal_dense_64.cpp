// Codegen probe: 64 static literals plus a wildcard.
//
// The cost of a dense literal match must not grow with the number
// of cases. If any function on the dispatch path stays out of line,
// the compiler writes all 64 cases to the stack on every call, which
// this probe catches as an instruction count far above the budget.
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
  return match(x)
         | on(PROBE_BLOCK_16(1),
              PROBE_BLOCK_16(17),
              PROBE_BLOCK_16(33),
              PROBE_BLOCK_16(49),
              _ >> 0);
}

#define REFERENCE_CASE(n)                                           \
  case (n):                                                         \
    return (n) * 3
#define REFERENCE_BLOCK_4(b)                                        \
  REFERENCE_CASE((b) + 0);                                          \
  REFERENCE_CASE((b) + 1);                                          \
  REFERENCE_CASE((b) + 2);                                          \
  REFERENCE_CASE((b) + 3)
#define REFERENCE_BLOCK_16(b)                                       \
  REFERENCE_BLOCK_4((b) + 0);                                       \
  REFERENCE_BLOCK_4((b) + 4);                                       \
  REFERENCE_BLOCK_4((b) + 8);                                       \
  REFERENCE_BLOCK_4((b) + 12)

// The same mapping written as a `switch`.
extern "C" int ptn_codegen_reference(int x) {
  switch (x) {
    REFERENCE_BLOCK_16(1);
    REFERENCE_BLOCK_16(17);
    REFERENCE_BLOCK_16(33);
    REFERENCE_BLOCK_16(49);
  default:
    return 0;
  }
}
