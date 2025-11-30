#pragma once

/**
 * @file relational.hpp
 * @brief Public API and implementation for relational comparison patterns.
 *
 * This file provides factory functions to create patterns that perform
 * relational comparisons (e.g., less-than, greater-than, equality). It includes
 * both the public API and the internal implementation details.
 *
 * @namespace ptn::pat::value
 */

#include <functional>
#include <type_traits>
#include <utility>
#include <tuple>

#include "ptn/pattern/base/fwd.h"
#include "ptn/config.hpp"

namespace ptn::pat::value {

  // --- Internal Implementation Details ---

  namespace detail {

    /**
     * @brief Internal storage type for relational values.
     *
     * Currently uses std::decay_t<T>, but may be extended for string_view
     * specialization in future releases.
     */
    template <typename T>
    using rel_store_t = std::decay_t<T>;

    /**
     * @brief Compile-time checks for values used in ordering relations.
     */
    template <typename T>
    constexpr void check_rel_value() {
      static_assert(
          !std::is_void_v<T>, "[Patternia.relational]: value cannot be void.");
      static_assert(
          !std::is_reference_v<T>,
          "[Patternia.relational]: value must be a value type.");
      static_assert(
          std::is_move_constructible_v<T>,
          "[Patternia.relational]: value must be move-constructible.");
      static_assert(
          std::is_invocable_r_v<bool, std::less<>, const T &, const T &>,
          "[Patternia.relational]: type must support operator< (std::less).");
    }

    // --- Pattern Implementations ---

    template <typename V, typename Cmp>
    struct rel_pattern_base : base::pattern_base<rel_pattern_base<V, Cmp>> {
      rel_store_t<V> v;
#if defined(__cpp_no_unique_address) && __cpp_no_unique_address >= 201803L
      [[no_unique_address]] Cmp cmp{};
#else
      Cmp cmp{};
#endif
      constexpr rel_pattern_base(rel_store_t<V> val, Cmp c = {})
          : v(std::move(val)), cmp(std::move(c)) {
      }

      template <typename X>
      constexpr bool match(X const &x) const noexcept(noexcept(cmp(x, v))) {
        return cmp(x, v);
      }

      template <typename X>
      constexpr auto bind(const X & /*subj*/) const {
        return std::tuple<>{};
      }
    };

    template <typename V>
    using lt_pattern = rel_pattern_base<V, std::less<>>;

    template <typename V>
    using le_pattern = rel_pattern_base<V, std::less_equal<>>;

    template <typename V>
    using gt_pattern = rel_pattern_base<V, std::greater<>>;

    template <typename V>
    using ge_pattern = rel_pattern_base<V, std::greater_equal<>>;

    template <typename V>
    using eq_pattern = rel_pattern_base<V, std::equal_to<>>;

    template <typename V>
    using ne_pattern = rel_pattern_base<V, std::not_equal_to<>>;

    /**
     * @brief Pattern checking that a subject value falls within an interval.
     */
    template <typename L, typename R, typename Cmp = std::less<>>
    struct between_pattern : base::pattern_base<between_pattern<L, R, Cmp>> {
      rel_store_t<L> lo;
      rel_store_t<R> hi;
      bool           closed{};
#if defined(__cpp_no_unique_address) && __cpp_no_unique_address >= 201803L
      [[no_unique_address]] Cmp cmp{};
#else
      Cmp cmp{};
#endif

      constexpr between_pattern(
          rel_store_t<L> lo_, rel_store_t<R> hi_, bool closed_, Cmp c = {})
          : lo(std::move(lo_)), hi(std::move(hi_)), closed(closed_),
            cmp(std::move(c)) {
      }

      template <typename X>
      constexpr bool match(X const &x) const noexcept(
          noexcept(std::declval<const Cmp &>()(x, lo)) &&
          noexcept(std::declval<const Cmp &>()(hi, x))) {
        if (closed) {
          return !cmp(x, lo) && !cmp(hi, x);
        }
        else {
          return cmp(lo, x) && cmp(x, hi);
        }
      }

      template <typename X>
      constexpr auto bind(const X & /*subj*/) const {
        return std::tuple<>{};
      }
    };

  } // namespace detail

  // --- Public API ---

  /// @brief Factory for less-than pattern `x < v`.
  template <typename V>
  constexpr auto lt(V &&v) {
    using store_t = detail::rel_store_t<V>;
    detail::check_rel_value<store_t>();
    return detail::lt_pattern<store_t>(store_t(std::forward<V>(v)));
  }

  /// @brief Factory for less-than-or-equal pattern `x <= v`.
  template <typename V>
  constexpr auto le(V &&v) {
    using store_t = detail::rel_store_t<V>;
    detail::check_rel_value<store_t>();
    return detail::le_pattern<store_t>(store_t(std::forward<V>(v)));
  }

  /// @brief Factory for greater-than pattern `x > v`.
  template <typename V>
  constexpr auto gt(V &&v) {
    using store_t = detail::rel_store_t<V>;
    detail::check_rel_value<store_t>();
    return detail::gt_pattern<store_t>(store_t(std::forward<V>(v)));
  }

  /// @brief Factory for greater-than-or-equal pattern `x >= v`.
  template <typename V>
  constexpr auto ge(V &&v) {
    using store_t = detail::rel_store_t<V>;
    detail::check_rel_value<store_t>();
    return detail::ge_pattern<store_t>(store_t(std::forward<V>(v)));
  }

  /// @brief Factory for equality pattern `x == v`.
  template <typename V>
  constexpr auto eq(V &&v) {
    using store_t = detail::rel_store_t<V>;
    static_assert(
        !std::is_void_v<store_t>, "[Patternia.eq]: value cannot be void.");
    static_assert(
        !std::is_reference_v<store_t>,
        "[Patternia.eq]: value must be a value type.");
    static_assert(
        std::is_move_constructible_v<store_t>,
        "[Patternia.eq]: value type must be move-constructible.");
    static_assert(
        std::is_invocable_r_v<
            bool,
            std::equal_to<>,
            const store_t &,
            const store_t &>,
        "[Patternia.eq]: value type must support operator== (std::equal_to).");
    return detail::eq_pattern<store_t>(store_t(std::forward<V>(v)));
  }

  /// @brief Factory for inequality pattern `x != v`.
  template <typename V>
  constexpr auto ne(V &&v) {
    using store_t = detail::rel_store_t<V>;
    static_assert(
        !std::is_void_v<store_t>, "[Patternia.ne]: value cannot be void.");
    static_assert(
        !std::is_reference_v<store_t>,
        "[Patternia.ne]: value must be a value type.");
    static_assert(
        std::is_move_constructible_v<store_t>,
        "[Patternia.ne]: value type must be move-constructible.");
    static_assert(
        std::is_invocable_r_v<
            bool,
            std::not_equal_to<>,
            const store_t &,
            const store_t &>,
        "[Patternia.ne]: value type must support operator!= "
        "(std::not_equal_to).");
    return detail::ne_pattern<store_t>(store_t(std::forward<V>(v)));
  }

  /// @brief Factory for interval pattern `[lo, hi]` or `(lo, hi)`.
  template <typename L, typename R>
  constexpr auto between(L &&lo, R &&hi, bool closed = true) {
    using lo_t = detail::rel_store_t<L>;
    using hi_t = detail::rel_store_t<R>;
    detail::check_rel_value<lo_t>();
    detail::check_rel_value<hi_t>();
    static_assert(
        std::is_invocable_r_v<bool, std::less<>, const lo_t &, const lo_t &> &&
            std::
                is_invocable_r_v<bool, std::less<>, const hi_t &, const hi_t &>,
        "[Patternia.between]: bounds must be comparable via std::less<>.");
    return detail::between_pattern<lo_t, hi_t>(
        lo_t(std::forward<L>(lo)), hi_t(std::forward<R>(hi)), closed);
  }

} // namespace ptn::pat::value

// --- Binding Contract Declarations ---

namespace ptn::pat::base {

  // --- for relational patterns ---
  template <typename V, typename Subject>
  struct binding_args<ptn::pat::value::detail::lt_pattern<V>, Subject> {
    using type = std::tuple<>;
  };

  template <typename V, typename Subject>
  struct binding_args<ptn::pat::value::detail::le_pattern<V>, Subject> {
    using type = std::tuple<>;
  };

  template <typename V, typename Subject>
  struct binding_args<ptn::pat::value::detail::gt_pattern<V>, Subject> {
    using type = std::tuple<>;
  };

  template <typename V, typename Subject>
  struct binding_args<ptn::pat::value::detail::ge_pattern<V>, Subject> {
    using type = std::tuple<>;
  };

  template <typename V, typename Subject>
  struct binding_args<ptn::pat::value::detail::eq_pattern<V>, Subject> {
    using type = std::tuple<>;
  };

  template <typename V, typename Subject>
  struct binding_args<ptn::pat::value::detail::ne_pattern<V>, Subject> {
    using type = std::tuple<>;
  };

  // --- for between_pattern ---
  template <typename L, typename R, typename Subject>
  struct binding_args<ptn::pat::value::detail::between_pattern<L, R>, Subject> {
    using type = std::tuple<>;
  };

} // namespace ptn::pat::base
