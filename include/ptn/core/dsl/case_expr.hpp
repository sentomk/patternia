#ifndef PTN_DSL_CASE_EXPR_HPP
#define PTN_DSL_CASE_EXPR_HPP

#include <utility>

namespace ptn::dsl {
  // case_expr: saving pattern and handler
  template <typename Pattern, typename Handler>
  struct case_expr {
    Pattern pattern;
    Handler handler;
  };

  // overload operator>>
  template <typename P, typename H>
  constexpr auto operator>>(P &&p, H &&h) {
    return case_expr<std::decay_t<P>, std::decay_t<H>>{
        std::forward<P>(p), std::forward<H>(h)};
  }

} // namespace ptn::dsl

#endif // PTN_DSL_CASE_EXPR_HPP