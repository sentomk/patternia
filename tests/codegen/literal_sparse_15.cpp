// Codegen probe: fifteen sparse runtime literals plus a wildcard.
//
// `lit(...)` values are not known to the planner, so the match picks
// the first case whose value equals the subject. Once inlined, every
// test compares the subject with a constant and every case returns a
// constant, which compilers lower like the `switch` below: a lookup
// table, a jump table or a few comparisons. Before the case pack was
// stored in an always-inlined case_pack, GCC lowered this match to a
// chain of comparisons in some translation units.
// See scripts/check_codegen.py.

#include <ptn/patternia.hpp>

extern "C" int ptn_codegen_probe(int x) {
  using namespace ptn;
  return match(x)
         | on(lit(0) >> 1,
              lit(7) >> 2,
              lit(14) >> 3,
              lit(21) >> 4,
              lit(28) >> 5,
              lit(35) >> 6,
              lit(42) >> 7,
              lit(49) >> 8,
              lit(56) >> 9,
              lit(63) >> 10,
              lit(70) >> 11,
              lit(77) >> 12,
              lit(84) >> 13,
              lit(91) >> 14,
              lit(98) >> 15,
              _ >> 0);
}

extern "C" int ptn_codegen_reference(int x) {
  switch (x) {
  case 0:
    return 1;
  case 7:
    return 2;
  case 14:
    return 3;
  case 21:
    return 4;
  case 28:
    return 5;
  case 35:
    return 6;
  case 42:
    return 7;
  case 49:
    return 8;
  case 56:
    return 9;
  case 63:
    return 10;
  case 70:
    return 11;
  case 77:
    return 12;
  case 84:
    return 13;
  case 91:
    return 14;
  case 98:
    return 15;
  default:
    return 0;
  }
}
