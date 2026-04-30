// compile-time literal-match benchmark – forces full instantiation
// of the match-dispatch machinery with 16 literal cases.

#include "ptn/patternia.hpp"

constexpr int ct_lit_16(int v) noexcept {
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
      __ >> 0);
}

static_assert(ct_lit_16(0) == 0, "");
static_assert(ct_lit_16(8) == 8, "");
static_assert(ct_lit_16(15) == 15, "");
static_assert(ct_lit_16(-1) == 0, "");
static_assert(ct_lit_16(16) == 0, "");
