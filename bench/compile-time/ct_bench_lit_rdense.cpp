#include "ptn/patternia.hpp"

namespace {
  constexpr int ct_lit_rdense(int v) noexcept {
    using namespace ptn;
    return match(v)
           | on(ptn::lit(0) >> 0,
                ptn::lit(7) >> 7,
                ptn::lit(14) >> 14,
                ptn::lit(21) >> 21,
                ptn::lit(28) >> 28,
                ptn::lit(35) >> 35,
                ptn::lit(42) >> 42,
                ptn::lit(49) >> 49,
                ptn::lit(56) >> 56,
                ptn::lit(63) >> 63,
                ptn::lit(70) >> 70,
                ptn::lit(77) >> 77,
                ptn::lit(84) >> 84,
                ptn::lit(91) >> 91,
                ptn::lit(98) >> 98,
                _ >> 0);
  }

  static_assert(ct_lit_rdense(0) == 0, "");
  static_assert(ct_lit_rdense(49) == 49, "");
  static_assert(ct_lit_rdense(98) == 98, "");
  static_assert(ct_lit_rdense(-1) == 0, "");
  static_assert(ct_lit_rdense(99) == 0, "");
  static_assert(ct_lit_rdense(35) == 35, "");
} // namespace
