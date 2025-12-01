#pragma once

/**
 * @file case_expr_impl.hpp (detail)
 * @brief Internal implementation of case expressions.
 *
 * @internal This is an internal implementation detail.
 */

namespace ptn::core::dsl::detail {

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
    using pattern_type = Pattern;
    using handler_type = Handler;

    Pattern pattern; ///< The pattern to match against
    Handler handler; ///< The handler to execute if pattern matches
  };

} // namespace ptn::core::dsl::detail
