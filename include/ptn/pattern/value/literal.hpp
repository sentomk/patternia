#pragma once

#include <utility>

#include "ptn/pattern/value/detail/literal_pattern.hpp"
#include "ptn/config.hpp"

/**
 * @file literal.hpp
 * @brief Public API for literal value patterns (`lit()` and `lit_ci()`).
 *
 * This module provides the public factory functions for creating literal
 * patterns. These patterns store a reference value and match subjects via
 * equality comparison.
 *
 * The actual implementation types (`literal_pattern<>`, `iequal_ascii`) are in
 * the `detail/` subdirectory and should not be used directly by end users.
 *
 * Part of the Pattern Layer (namespace ptn::pattern::value)
 */

namespace ptn::pattern::value {

  /**
   * @brief Factory function that constructs a value-pattern.
   *
   * Automatically adapts C-style strings into `std::string_view`, otherwise
   * stores values as `std::decay_t<V>`.
   *
   * @tparam V Input value type (deduced).
   * @param v  Value to match.
   * @return A `literal_pattern` configured for equality matching.
   *
   * @example
   * ```cpp
   * auto p = lit(42);
   * auto p_str = lit("hello");
   * auto matches = ptn::match(42).when(p >> "matched").otherwise("no");
   * ```
   */
  template <typename V>
  constexpr auto lit(V &&v) {
    using store_t = detail::literal_store_t<V>;
    return detail::literal_pattern<store_t>(store_t(std::forward<V>(v)));
  }

  /**
   * @brief Case-insensitive value-pattern factory.
   *
   * Used for matching command strings, tokens, or user input without
   * case sensitivity. Performs ASCII-only case folding.
   *
   * @tparam V Input value type (deduced).
   * @param v  Value to store.
   * @return A `literal_pattern` using `iequal_ascii` comparator.
   *
   * @example
   * ```cpp
   * auto p = lit_ci("HELLO");
   * auto matches = ptn::match("hello").when(p >> "matched").otherwise("no");
   * ```
   */
  template <typename V>
  constexpr auto lit_ci(V &&v) {
    using store_t = detail::literal_store_t<V>;
    return detail::literal_pattern<store_t, detail::iequal_ascii>(
        store_t(std::forward<V>(v)), detail::iequal_ascii{});
  }

} // namespace ptn::pattern::value
