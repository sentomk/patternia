#pragma once

#include <utility>
#include "ptn/pattern/value/detail/relational_pattern.hpp"
#include "ptn/config.hpp"

/**
 * @file relational.hpp
 * @brief Public API for relational comparison patterns.
 *
 * This module provides the public factory functions for creating relational
 * patterns: `lt()`, `le()`, `gt()`, `ge()`, `eq()`, `ne()`, and `between()`.
 *
 * All relational patterns compare a subject value against a stored value using
 * a comparator (default: `std::less<>` for relational, `std::equal_to<>` for
 * equality).
 *
 * The actual implementation types are in the `detail/` subdirectory and should
 * not be used directly by end users.
 *
 * Part of Pattern Layer (namespace ptn::pattern::value)
 */

namespace ptn::pattern::value {

  /**
   * @brief Factory for less-than pattern `x < v`.
   * @tparam V Value type (deduced).
   * @param v  The threshold value.
   * @return A pattern matching when subject is less than `v`.
   */
  template <typename V>
  constexpr auto lt(V &&v) {
    using store_t = detail::rel_store_t<V>;
    return detail::lt_pattern<store_t>(store_t(std::forward<V>(v)));
  }

  /**
   * @brief Factory for less-than-or-equal pattern `x <= v`.
   * @tparam V Value type (deduced).
   * @param v  The threshold value.
   * @return A pattern matching when subject is less than or equal to `v`.
   */
  template <typename V>
  constexpr auto le(V &&v) {
    using store_t = detail::rel_store_t<V>;
    return detail::le_pattern<store_t>(store_t(std::forward<V>(v)));
  }

  /**
   * @brief Factory for greater-than pattern `x > v`.
   * @tparam V Value type (deduced).
   * @param v  The threshold value.
   * @return A pattern matching when subject is greater than `v`.
   */
  template <typename V>
  constexpr auto gt(V &&v) {
    using store_t = detail::rel_store_t<V>;
    return detail::gt_pattern<store_t>(store_t(std::forward<V>(v)));
  }

  /**
   * @brief Factory for greater-than-or-equal pattern `x >= v`.
   * @tparam V Value type (deduced).
   * @param v  The threshold value.
   * @return A pattern matching when subject is greater than or equal to `v`.
   */
  template <typename V>
  constexpr auto ge(V &&v) {
    using store_t = detail::rel_store_t<V>;
    return detail::ge_pattern<store_t>(store_t(std::forward<V>(v)));
  }

  /**
   * @brief Factory for equality pattern `x == v`.
   * @tparam V Value type (deduced).
   * @param v  The value to match.
   * @return A pattern matching when subject equals `v`.
   */
  template <typename V>
  constexpr auto eq(V &&v) {
    using store_t = detail::rel_store_t<V>;
    return detail::eq_pattern<store_t>(store_t(std::forward<V>(v)));
  }

  /**
   * @brief Factory for inequality pattern `x != v`.
   * @tparam V Value type (deduced).
   * @param v  The value to exclude.
   * @return A pattern matching when subject does not equal `v`.
   */
  template <typename V>
  constexpr auto ne(V &&v) {
    using store_t = detail::rel_store_t<V>;
    return detail::ne_pattern<store_t>(store_t(std::forward<V>(v)));
  }

  /**
   * @brief Factory for interval pattern `[lo, hi]` or `(lo, hi)`.
   *
   * Creates a pattern that checks if a subject falls within a range.
   *
   * @tparam L Lower-bound type (deduced).
   * @tparam R Upper-bound type (deduced).
   * @param lo       Lower bound.
   * @param hi       Upper bound.
   * @param closed   If true (default), interval is closed `[lo, hi]`.
   *                 If false, interval is open `(lo, hi)`.
   * @return A pattern matching when subject is in the specified range.
   *
   * @example
   * ```cpp
   * auto p = between(0, 100);        // [0, 100]
   * auto p2 = between(0, 100, false); // (0, 100)
   * ```
   */
  template <typename L, typename R>
  constexpr auto between(L &&lo, R &&hi, bool closed = true) {
    using lo_store_t = detail::rel_store_t<L>;
    using hi_store_t = detail::rel_store_t<R>;
    return detail::between_pattern<lo_store_t, hi_store_t>(
        lo_store_t(std::forward<L>(lo)),
        hi_store_t(std::forward<R>(hi)),
        closed);
  }

} // namespace ptn::pattern::value
