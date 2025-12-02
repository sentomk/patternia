#pragma once

// Public type alias for case expressions.

namespace ptn::core::dsl::detail {
  template <typename Pattern, typename Handler>
  struct case_expr;
}

namespace ptn::core::dsl {

  // Public type alias for case expressions.
  //
  // Represents a pattern-handler pair in the match DSL.
  template <typename Pattern, typename Handler>
  using case_expr = detail::case_expr<Pattern, Handler>;

} // namespace ptn::core::dsl
