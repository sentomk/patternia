#pragma once

#include <utility>
#include "ptn/pattern/value/detail/relational_pattern.hpp"
#include "ptn/config.hpp"

/**
 * @file relational.hpp
 * @brief Public API for relational comparison patterns.
 */

namespace ptn::pattern::value {

  /**
   * @brief Factory for less-than pattern `x < v`.
   */
  template <typename V>
  constexpr auto lt(V &&v) {
    using store_t = detail::rel_store_t<V>;
    detail::check_rel_value<store_t>();
    return detail::lt_pattern<store_t>(store_t(std::forward<V>(v)));
  }

  /**
   * @brief Factory for less-than-or-equal pattern `x <= v`.
   */
  template <typename V>
  constexpr auto le(V &&v) {
    using store_t = detail::rel_store_t<V>;
    detail::check_rel_value<store_t>();
    return detail::le_pattern<store_t>(store_t(std::forward<V>(v)));
  }

  /**
   * @brief Factory for greater-than pattern `x > v`.
   */
  template <typename V>
  constexpr auto gt(V &&v) {
    using store_t = detail::rel_store_t<V>;
    detail::check_rel_value<store_t>();
    return detail::gt_pattern<store_t>(store_t(std::forward<V>(v)));
  }

  /**
   * @brief Factory for greater-than-or-equal pattern `x >= v`.
   */
  template <typename V>
  constexpr auto ge(V &&v) {
    using store_t = detail::rel_store_t<V>;
    detail::check_rel_value<store_t>();
    return detail::ge_pattern<store_t>(store_t(std::forward<V>(v)));
  }

  /**
   * @brief Factory for equality pattern `x == v`.
   */
  template <typename V>
  constexpr auto eq(V &&v) {
    using store_t = detail::rel_store_t<V>;

    static_assert(!std::is_void_v<store_t>, "[ptn.eq]: value cannot be void.");
    static_assert(
        !std::is_reference_v<store_t>, "[ptn.eq]: value must be a value type.");
    static_assert(
        std::is_move_constructible_v<store_t>,
        "[ptn.eq]: value type must be move-constructible.");
    static_assert(
        std::is_invocable_r_v<
            bool,
            std::equal_to<>,
            const store_t &,
            const store_t &>,
        "[ptn.eq]: value type must support operator== (std::equal_to).");

    return detail::eq_pattern<store_t>(store_t(std::forward<V>(v)));
  }
  /**
   * @brief Factory for inequality pattern `x != v`.
   */
  template <typename V>
  constexpr auto ne(V &&v) {
    using store_t = detail::rel_store_t<V>;

    static_assert(!std::is_void_v<store_t>, "[ptn.ne]: value cannot be void.");
    static_assert(
        !std::is_reference_v<store_t>, "[ptn.ne]: value must be a value type.");
    static_assert(
        std::is_move_constructible_v<store_t>,
        "[ptn.ne]: value type must be move-constructible.");
    static_assert(
        std::is_invocable_r_v<
            bool,
            std::not_equal_to<>,
            const store_t &,
            const store_t &>,
        "[ptn.ne]: value type must support operator!= (std::not_equal_to).");

    return detail::ne_pattern<store_t>(store_t(std::forward<V>(v)));
  }
  /**
   * @brief Factory for interval pattern `[lo, hi]` or `(lo, hi)`.
   *
   * Creates a pattern that checks if a subject falls within a range.
   */
  template <typename L, typename R>
  constexpr auto between(L &&lo, R &&hi, bool closed = true) {
    using lo_t = detail::rel_store_t<L>;
    using hi_t = detail::rel_store_t<R>;

    // ---- lower-bound checks ----
    detail::check_rel_value<lo_t>();
    // ---- upper-bound checks ----
    detail::check_rel_value<hi_t>();

    // ---- ensure both bounds are comparable types ----
    static_assert(
        std::is_invocable_r_v<bool, std::less<>, const lo_t &, const lo_t &> &&
            std::
                is_invocable_r_v<bool, std::less<>, const hi_t &, const hi_t &>,
        "[ptn.between]: bounds must be comparable via std::less<>.");

    return detail::between_pattern<lo_t, hi_t>(
        lo_t(std::forward<L>(lo)), hi_t(std::forward<R>(hi)), closed);
  }
} // namespace ptn::pattern::value
