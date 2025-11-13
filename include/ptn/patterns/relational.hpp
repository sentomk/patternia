#pragma once

#include <functional>  // std::less<>
#include <type_traits> // std::decay_t
#include <utility>     // std::forward

#include "ptn/patterns/pattern_base.hpp"
#include "ptn/config.hpp"

/**
 * @file relational.hpp
 * @brief Relational comparison patterns: < <= > >= == != and between.
 *
 * All relational patterns compare a subject value against a stored value using
 * a comparator (default: `std::less<>` or `std::equal_to<>`).
 *
 * These patterns inherit from pattern_base and therefore supply:
 *   - `match(subject)` for matching
 *   - default identity `bind(subject)` unless overridden
 *
 * Example:
 * @code {.cpp}
 * using namespace ptn::patterns;
 *
 * match(x)
 *   .when(lt(10) >> "small")
 *   .when(between(10, 20) >> "medium")
 *   .when(ge(20) >> "large");
 * @endcode
 *
 * @ingroup patterns
 */

namespace ptn::patterns {

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
   */
  template <typename V, typename Cmp = std::less<>>
  struct lt_pattern : pattern_base<lt_pattern<V, Cmp>> {
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
  template <typename V, typename Cmp = std::less<>>
  struct le_pattern : pattern_base<le_pattern<V, Cmp>> {
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
  template <typename V, typename Cmp = std::less<>>
  struct gt_pattern : pattern_base<gt_pattern<V, Cmp>> {
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
  template <typename V, typename Cmp = std::less<>>
  struct ge_pattern : pattern_base<ge_pattern<V, Cmp>> {
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
  template <typename V, typename Cmp = std::equal_to<>>
  struct eq_pattern : pattern_base<eq_pattern<V, Cmp>> {
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
  template <typename V, typename Cmp = std::not_equal_to<>>
  struct ne_pattern : pattern_base<ne_pattern<V, Cmp>> {
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

  // Factories
  template <typename V>
  constexpr auto lt(V &&v) {
    return lt_pattern<rel_store_t<V>>(rel_store_t<V>(std::forward<V>(v)));
  }

  template <typename V>
  constexpr auto le(V &&v) {
    return le_pattern<rel_store_t<V>>(rel_store_t<V>(std::forward<V>(v)));
  }

  template <typename V>
  constexpr auto gt(V &&v) {
    return gt_pattern<rel_store_t<V>>(rel_store_t<V>(std::forward<V>(v)));
  }

  template <typename V>
  constexpr auto ge(V &&v) {
    return ge_pattern<rel_store_t<V>>(rel_store_t<V>(std::forward<V>(v)));
  }

  template <typename V>
  constexpr auto eq(V &&v) {
    return eq_pattern<rel_store_t<V>>(rel_store_t<V>(std::forward<V>(v)));
  }

  template <typename V>
  constexpr auto ne(V &&v) {
    return ne_pattern<rel_store_t<V>>(rel_store_t<V>(std::forward<V>(v)));
  }

  //
  //  between:
  //  closed==true  -> [lo, hi] :  !(x < lo) && !(hi < x)
  //  closed==false -> (lo, hi) :  (lo < x) &&  (x < hi)
  //

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
   */
  template <typename L, typename R, typename Cmp = std::less<>>
  struct between_pattern : pattern_base<between_pattern<L, R, Cmp>> {
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

  template <typename L, typename R>
  constexpr auto between(L &&lo, R &&hi, bool closed = true) {
    return between_pattern<rel_store_t<L>, rel_store_t<R>>(
        rel_store_t<L>(std::forward<L>(lo)),
        rel_store_t<R>(std::forward<R>(hi)),
        closed);
  }
} // namespace ptn::patterns
