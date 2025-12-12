#pragma once

// Public type alias for case expressions.
//
// This header provides the public interface for case expressions used in
// Patternia's domain-specific language for pattern matching.

namespace ptn::core::dsl::detail {
  /// Forward declaration of internal case expression implementation
  template <typename Pattern, typename Handler>
  struct case_expr;
}

namespace ptn::core::dsl {

  /// Public type alias for case expressions
  /// Represents a pattern-handler pair in the match DSL
  /// Created using the >> operator: pattern >> handler
  template <typename Pattern, typename Handler>
  using case_expr = detail::case_expr<Pattern, Handler>;

} // namespace ptn::core::dsl
