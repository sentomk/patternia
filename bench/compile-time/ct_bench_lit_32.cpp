// compile-time literal-match benchmark – forces full instantiation
// of the match-dispatch machinery with 32 literal cases.

#include "ptn/patternia.hpp"

constexpr int ct_lit_32(int v) noexcept {
  using namespace ptn;
  return match(v) | on(
      ptn::lit(0) >> 0,
      ptn::lit(1) >> 1,
      ptn::lit(2) >> 2,
      ptn::lit(3) >> 3,
      ptn::lit(4) >> 4,
      ptn::lit(5) >> 5,
      ptn::lit(6) >> 6,
      ptn::lit(7) >> 7,
      ptn::lit(8) >> 8,
      ptn::lit(9) >> 9,
      ptn::lit(10) >> 10,
      ptn::lit(11) >> 11,
      ptn::lit(12) >> 12,
      ptn::lit(13) >> 13,
      ptn::lit(14) >> 14,
      ptn::lit(15) >> 15,
      ptn::lit(16) >> 16,
      ptn::lit(17) >> 17,
      ptn::lit(18) >> 18,
      ptn::lit(19) >> 19,
      ptn::lit(20) >> 20,
      ptn::lit(21) >> 21,
      ptn::lit(22) >> 22,
      ptn::lit(23) >> 23,
      ptn::lit(24) >> 24,
      ptn::lit(25) >> 25,
      ptn::lit(26) >> 26,
      ptn::lit(27) >> 27,
      ptn::lit(28) >> 28,
      ptn::lit(29) >> 29,
      ptn::lit(30) >> 30,
      ptn::lit(31) >> 31,
      __ >> 0);
}

static_assert(ct_lit_32(0) == 0, "");
static_assert(ct_lit_32(16) == 16, "");
static_assert(ct_lit_32(31) == 31, "");
static_assert(ct_lit_32(-1) == 0, "");
static_assert(ct_lit_32(32) == 0, "");
