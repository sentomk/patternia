// compile-time literal-match benchmark – forces full instantiation
// of the match-dispatch machinery with 8 literal cases.

#include "ptn/patternia.hpp"

constexpr int ct_lit_8(int v) noexcept {
  using namespace ptn;
  return match(v)
         | on(ptn::lit(0) >> 0,
              ptn::lit(1) >> 1,
              ptn::lit(2) >> 2,
              ptn::lit(3) >> 3,
              ptn::lit(4) >> 4,
              ptn::lit(5) >> 5,
              ptn::lit(6) >> 6,
              ptn::lit(7) >> 7,
              __ >> 0);
}

static_assert(ct_lit_8(0) == 0, "");
static_assert(ct_lit_8(4) == 4, "");
static_assert(ct_lit_8(7) == 7, "");
static_assert(ct_lit_8(-1) == 0, "");
static_assert(ct_lit_8(8) == 0, "");
