#pragma once

#include <functional>  // std::less<>
#include <type_traits> // std::decay_t
#include <utility>     // std::forward

#include "ptn/pattern/detail/pattern_base.hpp"
#include "ptn/config.hpp"

/**
 * @file relational.hpp (detail)
 * @brief Internal implementation of relational comparison patterns.
 *
 * This is an internal header containing the implementation of relational
 * patterns: `lt_pattern<>`, `le_pattern<>`, `gt_pattern<>`, `ge_pattern<>`,
 * `eq_pattern<>`, `ne_pattern<>`, and `between_pattern<>`.
 *
 * Users should use the public factory functions (`lt()`, `le()`, `gt()`,
 * `ge()`, `eq()`, `ne()`, `between()`) from `ptn/pattern/value/relational.hpp`
 * instead of directly instantiating these types.
 *
 * Part of the Pattern Layer (namespace ptn::pattern::value::detail)
 */

namespace ptn::pattern::value::detail {

  /**
   * @brief Internal storage type for relational values.
   *
   * Currently uses std::decay_t<T>, but may be extended for string_view
   * specialization in future releases.
   */
  template <typename T>
  using rel_store_t = std::decay_t<T>;

  // less than
  /**
   * @brief Pattern matching `x < v`.
   *
   * @tparam V   value type
   * @tparam Cmp comparator type (default: `std::less<>`)
   *
   * @internal This is an internal implementation detail. Use `lt()` factory.
   */
  template <typename V, typename Cmp = std::less<>>
  struct lt_pattern : pattern::detail::pattern_base<lt_pattern<V, Cmp>> {
    rel_store_t<V> v;
#if defined(__cpp_no_unique_address) && __cpp_no_unique_address >= 201803L
    [[no_unique_address]] Cmp cmp{};
#else
    Cmp cmp{};
#endif

    constexpr lt_pattern(rel_store_t<V> val, Cmp c = {})
        : v(std::move(val)), cmp(std::move(c)) {
    }
    template <typename X>
    constexpr bool match(X const &x) const
        noexcept(noexcept(std::declval<const Cmp &>()(x, v))) {
      return cmp(x, v);
    }
    template <typename X>
    constexpr bool operator()(X const &x) const
        noexcept(noexcept(this->match(x))) {
      return this->match(x);
    }
  };

  // x <= v  <=>  !(v < x)
  /**
   * @brief Pattern matching `x <= v`.
   *
   * @tparam V   value type
   * @tparam Cmp comparator type (default: `std::less<>`)
   *
   * @internal This is an internal implementation detail. Use `le()` factory.
   */
  template <typename V, typename Cmp = std::less<>>
  struct le_pattern : pattern::detail::pattern_base<le_pattern<V, Cmp>> {
    rel_store_t<V> v;
#if defined(__cpp_no_unique_address) && __cpp_no_unique_address >= 201803L
    [[no_unique_address]] Cmp cmp{};
#else
    Cmp cmp{};
#endif

    constexpr le_pattern(rel_store_t<V> val, Cmp c = {})
        : v(std::move(val)), cmp(std::move(c)) {
    }
    template <typename X>
    constexpr bool match(X const &x) const
        noexcept(noexcept(std::declval<const Cmp &>()(v, x))) {
      return !cmp(v, x);
    }
    template <typename X>
    constexpr bool operator()(X const &x) const
        noexcept(noexcept(this->match(x))) {
      return this->match(x);
    }
  };

  // x > v  <=>  (v < x)
  /**
   * @brief Pattern matching `x > v`.
   *
   * @tparam V   value type
   * @tparam Cmp comparator type (default: `std::less<>`)
   *
   * @internal This is an internal implementation detail. Use `gt()` factory.
   */
  template <typename V, typename Cmp = std::less<>>
  struct gt_pattern : pattern::detail::pattern_base<gt_pattern<V, Cmp>> {
    rel_store_t<V> v;
#if defined(__cpp_no_unique_address) && __cpp_no_unique_address >= 201803L
    [[no_unique_address]] Cmp cmp{};
#else
    Cmp cmp{};
#endif

    constexpr gt_pattern(rel_store_t<V> val, Cmp c = {})
        : v(std::move(val)), cmp(std::move(c)) {
    }
    template <typename X>
    constexpr bool match(X const &x) const
        noexcept(noexcept(std::declval<const Cmp &>()(v, x))) {
      return cmp(v, x);
    }
    template <typename X>
    constexpr bool operator()(X const &x) const
        noexcept(noexcept(this->match(x))) {
      return this->match(x);
    }
  };

  // x >= v  <=>  !(x < v)
  /**
   * @brief Pattern matching `x >= v`.
   *
   * @tparam V   value type
   * @tparam Cmp comparator type (default: `std::less<>`)
   *
   * @internal This is an internal implementation detail. Use `ge()` factory.
   */
  template <typename V, typename Cmp = std::less<>>
  struct ge_pattern : pattern::detail::pattern_base<ge_pattern<V, Cmp>> {
    rel_store_t<V> v;
#if defined(__cpp_no_unique_address) && __cpp_no_unique_address >= 201803L
    [[no_unique_address]] Cmp cmp{};
#else
    Cmp cmp{};
#endif
    constexpr ge_pattern(rel_store_t<V> val, Cmp c = {})
        : v(std::move(val)), cmp(std::move(c)) {
    }
    template <typename X>
    constexpr bool match(X const &x) const
        noexcept(noexcept(std::declval<const Cmp &>()(x, v))) {
      return !cmp(x, v);
    }
    template <typename X>
    constexpr bool operator()(X const &x) const
        noexcept(noexcept(this->match(x))) {
      return this->match(x);
    }
  };

  // x == v
  /**
   * @brief Pattern matching `x == v`.
   *
   * @tparam V   value type
   * @tparam Cmp comparator type (default: `std::equal_to<>`)
   *
   * @internal This is an internal implementation detail. Use `eq()` factory.
   */
  template <typename V, typename Cmp = std::equal_to<>>
  struct eq_pattern : pattern::detail::pattern_base<eq_pattern<V, Cmp>> {
    rel_store_t<V> v;
#if defined(__cpp_no_unique_address) && __cpp_no_unique_address >= 201803L
    [[no_unique_address]] Cmp cmp{};
#else
    Cmp cmp{};
#endif

    constexpr eq_pattern(rel_store_t<V> val, Cmp c = {})
        : v(std::move(val)), cmp(std::move(c)) {
    }
    template <typename X>
    constexpr bool match(X const &x) const
        noexcept(noexcept(std::declval<const Cmp &>()(x, v))) {
      return cmp(x, v);
    }
    template <typename X>
    constexpr bool operator()(X const &x) const
        noexcept(noexcept(this->match(x))) {
      return this->match(x);
    }
  };

  // x != v
  /**
   * @brief Pattern matching `x != v`.
   *
   * @tparam V   value type
   * @tparam Cmp comparator type (default: `std::not_equal_to<>`)
   *
   * @internal This is an internal implementation detail. Use `ne()` factory.
   */
  template <typename V, typename Cmp = std::not_equal_to<>>
  struct ne_pattern : pattern::detail::pattern_base<ne_pattern<V, Cmp>> {
    rel_store_t<V> v;
#if defined(__cpp_no_unique_address) && __cpp_no_unique_address >= 201803L
    [[no_unique_address]] Cmp cmp{};
#else
    Cmp cmp{};
#endif

    constexpr ne_pattern(rel_store_t<V> val, Cmp c = {})
        : v(std::move(val)), cmp(std::move(c)) {
    }
    template <typename X>
    constexpr bool match(X const &x) const
        noexcept(noexcept(std::declval<const Cmp &>()(x, v))) {
      return cmp(x, v);
    }
    template <typename X>
    constexpr bool operator()(X const &x) const
        noexcept(noexcept(this->match(x))) {
      return this->match(x);
    }
  };

  /**
   * @brief Pattern checking that a subject value falls within an interval.
   *
   * @tparam L   lower-bound type
   * @tparam R   upper-bound type
   * @tparam Cmp comparator (`std::less<>` by default)
   *
   * Closed interval:
   *   `[lo, hi]  → !(x < lo) && !(hi < x)`
   *
   * Open interval:
   *   `(lo, hi)  → (lo < x) && (x < hi)`
   *
   * @internal This is an internal implementation detail. Use `between()`
   * factory.
   */
  template <typename L, typename R, typename Cmp = std::less<>>
  struct between_pattern
      : pattern::detail::pattern_base<between_pattern<L, R, Cmp>> {
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
        noexcept(std::declval<const Cmp &>()(hi, x)) &&
        noexcept(std::declval<const Cmp &>()(lo, x)) &&
        noexcept(std::declval<const Cmp &>()(x, hi))) {
      if (closed) {
        return !cmp(x, lo) && !cmp(hi, x);
      }
      else {
        return cmp(lo, x) && cmp(x, hi);
      }
    }
    template <typename X>
    constexpr bool operator()(X const &x) const
        noexcept(noexcept(this->match(x))) {
      return this->match(x);
    }
  };

  // static_assert for relational patterns
  template <typename T>
  constexpr void check_rel_value() {
    static_assert(
        !std::is_void_v<T>, "[ptn.relational]: value cannot be void.");
    static_assert(
        !std::is_reference_v<T>,
        "[ptn.relational]: value must be a value type.");
    static_assert(
        std::is_move_constructible_v<T>,
        "[ptn.relational]: value must be move-constructible.");
    static_assert(
        std::is_invocable_r_v<bool, std::less<>, const T &, const T &>,
        "[ptn.relational]: type must support operator< (std::less).");
  }
} // namespace ptn::pattern::value::detail
