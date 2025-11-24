#pragma once

/**
 * @file case_expr_impl.hpp (detail)
 * @brief Internal implementation of case expressions.
 *
 * This is an internal header containing the implementation of:
 * - `case_expr<Pattern, Handler>`: Represents a single case in match() DSL.
 *
 * Users should create case expressions using the `>>` operator defined in
 * `ptn/core/dsl/operators.hpp` instead of directly instantiating this type.
 *
 * Part of the Core Layer (namespace ptn::dsl::detail)
 *
 * @internal This is an internal implementation detail.
 */

namespace ptn::dsl::detail {

  /**
   * @brief Internal representation of a pattern-handler pair (case).
   *
   * This struct is created implicitly when using the `>>` operator:
   * ```cpp
   * pattern >> handler  // Creates a case_expr internally
   * ```
   *
   * @tparam Pattern The pattern type.
   * @tparam Handler The handler type.
   *
   * @internal This is an internal implementation detail. Use `>>` operator.
   */
  template <typename Pattern, typename Handler>
  struct case_expr {
    Pattern pattern; ///< The pattern to match against
    Handler handler; ///< The handler to execute if pattern matches
  };

} // namespace ptn::dsl::detail
