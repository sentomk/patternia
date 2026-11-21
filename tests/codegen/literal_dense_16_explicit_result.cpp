// Codegen probe: 16 static literals plus a wildcard, with the result
// type given to on<R>.
//
// on<R> only changes the result type, so the match must compile
// like the same `switch`. See scripts/check_codegen.py.

#include <ptn/patternia.hpp>

#define PROBE_CASE(n) ptn::val<n> >> static_cast<short>((n) * 3)
#define PROBE_BLOCK_4(b)                                            \
  PROBE_CASE((b) + 0), PROBE_CASE((b) + 1), PROBE_CASE((b) + 2),    \
      PROBE_CASE((b) + 3)
#define PROBE_BLOCK_16(b)                                           \
  PROBE_BLOCK_4((b) + 0), PROBE_BLOCK_4((b) + 4),                   \
      PROBE_BLOCK_4((b) + 8), PROBE_BLOCK_4((b) + 12)

extern "C" long ptn_codegen_probe(int x) {
  using namespace ptn;
  return match(x) | on<long>(PROBE_BLOCK_16(1), _ >> 0);
}

#define REFERENCE_CASE(n)                                           \
  case (n):                                                         \
    return (n) * 3
#define REFERENCE_BLOCK_4(b)                                        \
  REFERENCE_CASE((b) + 0);                                          \
  REFERENCE_CASE((b) + 1);                                          \
  REFERENCE_CASE((b) + 2);                                          \
  REFERENCE_CASE((b) + 3)

// The same mapping written as a `switch`.
extern "C" long ptn_codegen_reference(int x) {
  switch (x) {
    REFERENCE_BLOCK_4(1);
    REFERENCE_BLOCK_4(5);
    REFERENCE_BLOCK_4(9);
    REFERENCE_BLOCK_4(13);
  default:
    return 0;
  }
}
