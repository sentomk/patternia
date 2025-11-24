#pragma once

#include <utility>

#include "ptn/pattern/value/detail/predicate_pattern.hpp"
#include "ptn/config.hpp"

/**
 * @file predicate.hpp
 * @brief Public API for predicate-based patterns and logical composition.
 *
 * This module provides the public factory function `pred()` for wrapping
 * arbitrary boolean callables as patterns.
 *
 * Logical composition patterns (`and_pattern`, `or_pattern`, `not_pattern`) are
 * created via DSL operators (`&&`, `||`, `!`) defined in `ptn/dsl/ops.hpp`.
 *
 * The actual implementation types are in the `detail/` subdirectory and should
 * not be used directly by end users.
 *
 * Part of the Pattern Layer (namespace ptn::pattern::value)
 */

namespace ptn::pattern::value {

  /**
   * @brief Factory for predicate-based patterns.
   *
   * Wraps any callable returning something convertible to `bool` as a pattern.
   * Matches when the predicate function returns `true`.
   *
   * @tparam F Callable type (deduced).
   * @param f  A callable of the form `bool(const T&)`.
   * @return A `predicate_pattern` wrapping the callable.
   *
   * @example
   * ```cpp
   * auto is_even = pred([](int x) { return x % 2 == 0; });
   * auto matches = ptn::match(42).when(is_even >> "even").otherwise("odd");
   * ```
   */
  template <typename F>
  constexpr auto pred(F &&f) {
    return detail::predicate_pattern<std::decay_t<F>>(std::forward<F>(f));
  }

} // namespace ptn::pattern::value