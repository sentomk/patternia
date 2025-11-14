#pragma once

#include <type_traits>
#include <utility>

#include "ptn/pattern/pattern_base.hpp"
#include "ptn/config.hpp"

/**
 * @file predicate.hpp
 * @brief Predicate-based patterns and logical composition (`&&`, `||`, `!`).
 *
 * This module belongs to the *Pattern Layer* and provides:
 *   - `predicate_pattern<F>`: wraps any boolean callable
 *   - `and_pattern`, `or_pattern`, `not_pattern`: Boolean combinators
 *   - DSL composition operators under `patterns::ops`
 *
 * All predicate-style patterns are *Filter Patterns*: they override `match()`
 * but leave `bind()` to the default identity implementation in pattern_base.
 *
 * Part of the Pattern Layer (namespace ptn::pattern::value)
 */

namespace ptn::pattern::value {

  // predicate_pattern: wraps any callable returning bool

  /**
   * @brief A pattern wrapping any callable returning something convertible to
   * bool.
   *
   * @tparam F A callable type that must support the expression `fn(x)`.
   */
  template <typename F>
  struct predicate_pattern : pattern::pattern_base<predicate_pattern<F>> {

#if defined(__cpp_no_unique_address) && __cpp_no_unique_address >= 201803L
    [[no_unique_address]] F fn;
#else
    F fn;
#endif

    constexpr explicit predicate_pattern(F f) : fn(std::move(f)) {
    }

    /**
     * @brief Match if `fn(x)` evaluates to true.
     */
    template <typename X>
    constexpr bool match(X const &x) const
        noexcept(noexcept(std::declval<const F &>(), x)) {
      return fn(x);
    }
    template <typename X>
    constexpr bool operator()(X const &x) const
        noexcept(noexcept(this->match(x))) {
      return this->match(x);
    }
  };

  /**
   * @brief Factory for predicate_pattern.
   */
  template <typename F>
  constexpr auto pred(F &&f) {
    return predicate_pattern<std::decay_t<F>>(std::forward<F>(f));
  }

  // logical and

  /**
   * @brief Logical AND composition of two patterns.
   *
   * `match(x)` succeeds if both `l.match(x)` and `r.match(x)` succeed.
   */
  template <typename L, typename R>
  struct and_pattern : pattern::pattern_base<and_pattern<L, R>> {

#if defined(__cpp_no_unique_address) && __cpp_no_unique_address >= 201803L
    [[no_unique_address]] L l;
    [[no_unique_address]] R r;
#else
    L l;
    R r;
#endif

    constexpr and_pattern(L lhs, R rhs) : l(std::move(lhs)), r(std::move(rhs)) {
    }
    template <typename X>
    constexpr bool match(X const &x) const
        noexcept(noexcept(l.match(x)) && noexcept(r.match(x))) {
      return l.match(x) && r.match(x);
    }
    template <typename X>
    constexpr bool operator()(X const &x) const
        noexcept(noexcept(this->match(x))) {
      return this->match(x);
    }
  };

  // logical or

  /**
   * @brief Logical OR composition of two patterns.
   *
   * `match(x)` succeeds iff either `l.match(x)` or `r.match(x)` succeeds.
   */
  template <typename L, typename R>
  struct or_pattern : pattern::pattern_base<or_pattern<L, R>> {

#if defined(__cpp_no_unique_address) && __cpp_no_unique_address >= 201803L
    [[no_unique_address]] L l;
    [[no_unique_address]] R r;
#else
    L l;
    R r;
#endif

    constexpr or_pattern(L lhs, R rhs) : l(std::move(lhs)), r(std::move(rhs)) {
    }
    template <typename X>
    constexpr bool match(X const &x) const
        noexcept(noexcept(l.match(x)) && noexcept(r.match(x))) {
      return l.match(x) || r.match(x);
    }
    template <typename X>
    constexpr bool operator()(X const &x) const
        noexcept(noexcept(this->match(x))) {
      return this->match(x);
    }
  };

  // not
  template <typename P>
  struct not_pattern : pattern::pattern_base<not_pattern<P>> {

#if defined(__cpp_no_unique_address) && __cpp_no_unique_address >= 201803L
    [[no_unique_address]] P p;
#else
    P p;
#endif

    constexpr explicit not_pattern(P inner) : p(std::move(inner)) {
    }
    template <typename X>
    constexpr bool match(X const &x) const noexcept(noexcept(!p.match(x))) {
      return !p.match(x);
    }
    template <typename X>
    constexpr bool operator()(X const &x) const
        noexcept(noexcept(this->match(x))) {
      return this->match(x);
    }
  };

} // namespace ptn::pattern::value