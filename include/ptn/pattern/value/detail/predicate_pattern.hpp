#pragma once

#include <utility>

#include "ptn/pattern/detail/pattern_base.hpp"
#include "ptn/config.hpp"

/**
 * @file predicate.hpp (detail)
 * @brief Internal implementation of predicate-based patterns and logical
 * composition.
 *
 * This is an internal header containing the implementation of:
 *   - `predicate_pattern<F>`: wraps any boolean callable
 *   - `and_pattern<L, R>`: Boolean AND composition
 *   - `or_pattern<L, R>`: Boolean OR composition
 *   - `not_pattern<P>`: Boolean NOT composition
 *
 * Users should use the public factory function `pred()` from
 * `ptn/pattern/value/predicate.hpp` instead of directly instantiating
 * `predicate_pattern`.
 *
 * Logical composition patterns are created via DSL operators (&&, ||, !)
 * defined in `ptn/dsl/ops.hpp`.
 *
 * Part of the Pattern Layer (namespace ptn::pattern::value::detail)
 */

namespace ptn::pattern::value::detail {

  // predicate_pattern: wraps any callable returning bool

  /**
   * @brief A pattern wrapping any callable returning something convertible to
   * bool.
   *
   * @tparam F A callable type that must support the expression `fn(x)`.
   *
   * @internal This is an internal implementation detail. Use `pred()` factory.
   */
  template <typename F>
  struct predicate_pattern
      : pattern::detail::pattern_base<predicate_pattern<F>> {

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

  // logical and

  /**
   * @brief Logical AND composition of two patterns.
   *
   * `match(x)` succeeds if both `l.match(x)` and `r.match(x)` succeed.
   *
   * @internal This is an internal implementation detail. Created via `&&`
   * operator.
   */
  template <typename L, typename R>
  struct and_pattern : pattern::detail::pattern_base<and_pattern<L, R>> {

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
   *
   * @internal This is an internal implementation detail. Created via `||`
   * operator.
   */
  template <typename L, typename R>
  struct or_pattern : pattern::detail::pattern_base<or_pattern<L, R>> {

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

  // logical not

  /**
   * @brief Logical NOT composition of a pattern.
   *
   * `match(x)` succeeds iff `p.match(x)` fails.
   *
   * @internal This is an internal implementation detail. Created via `!`
   * operator.
   */
  template <typename P>
  struct not_pattern : pattern::detail::pattern_base<not_pattern<P>> {

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

} // namespace ptn::pattern::value::detail
