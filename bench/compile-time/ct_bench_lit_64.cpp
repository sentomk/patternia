// compile-time literal-match benchmark – forces full instantiation
// of the match-dispatch machinery with 64 literal cases.

#include "ptn/patternia.hpp"

constexpr int ct_lit_64(int v) noexcept {
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
      ptn::lit(32) >> 32,
      ptn::lit(33) >> 33,
      ptn::lit(34) >> 34,
      ptn::lit(35) >> 35,
      ptn::lit(36) >> 36,
      ptn::lit(37) >> 37,
      ptn::lit(38) >> 38,
      ptn::lit(39) >> 39,
      ptn::lit(40) >> 40,
      ptn::lit(41) >> 41,
      ptn::lit(42) >> 42,
      ptn::lit(43) >> 43,
      ptn::lit(44) >> 44,
      ptn::lit(45) >> 45,
      ptn::lit(46) >> 46,
      ptn::lit(47) >> 47,
      ptn::lit(48) >> 48,
      ptn::lit(49) >> 49,
      ptn::lit(50) >> 50,
      ptn::lit(51) >> 51,
      ptn::lit(52) >> 52,
      ptn::lit(53) >> 53,
      ptn::lit(54) >> 54,
      ptn::lit(55) >> 55,
      ptn::lit(56) >> 56,
      ptn::lit(57) >> 57,
      ptn::lit(58) >> 58,
      ptn::lit(59) >> 59,
      ptn::lit(60) >> 60,
      ptn::lit(61) >> 61,
      ptn::lit(62) >> 62,
      ptn::lit(63) >> 63,
      __ >> 0);
}

static_assert(ct_lit_64(0) == 0, "");
static_assert(ct_lit_64(32) == 32, "");
static_assert(ct_lit_64(63) == 63, "");
static_assert(ct_lit_64(-1) == 0, "");
static_assert(ct_lit_64(64) == 0, "");
