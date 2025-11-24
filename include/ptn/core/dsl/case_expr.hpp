#pragma once

/**
 * @file case_expr.hpp
 * @brief Public type alias for case expressions.
 *
 * This module provides the public `case_expr` type (via alias to detail
 * implementation). Case expressions are created implicitly when using the `>>`
 * operator:
 *
 * ```cpp
 * auto case = pattern >> handler;  // Creates a case_expr<Pattern, Handler>
 * ```
 *
 * Users should NOT instantiate `case_expr` directly. Use the `>>` operator
 * instead.
 *
 * Part of the Core Layer (namespace ptn::dsl)
 */

#include "ptn/core/dsl/detail/case_expr_detail.hpp"

namespace ptn::dsl::detail {
  template <typename Pattern, typename Handler>
  struct case_expr;
}

namespace ptn::dsl {

  /**
   * @brief Public type alias for case expressions.
   *
   * Represents a pattern-handler pair in the match DSL.
   * Created via the `>>` operator: `pattern >> handler`.
   *
   * @tparam Pattern The pattern type.
   * @tparam Handler The handler type.
   *
   * @example
   * ```cpp
   * using namespace ptn::pattern::value;
   * auto case1 = lit(10) >> "ten";
   * auto case2 = lt(5) >> "small";
   * ```
   */
  template <typename Pattern, typename Handler>
  using case_expr = detail::case_expr<Pattern, Handler>;

} // namespace ptn::dsl