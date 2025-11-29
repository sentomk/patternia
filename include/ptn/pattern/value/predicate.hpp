#pragma once

/**
 * @file predicate.hpp
 * @brief Public API and implementation for predicate-based patterns and logical
 * composition.
 *
 * This file provides factory functions to create patterns that wrap arbitrary
 * predicates and to compose patterns using logical operators. It includes both
 * the public API and the internal implementation details.
 *
 * @namespace ptn::pat::value
 */

#include <utility>
#include <tuple>
#include <type_traits>

#include "ptn/pattern/base/fwd.h"
#include "ptn/config.hpp"

namespace ptn::pat::value {

  // --- Internal Implementation Details ---

  namespace detail {

    /**
     * @brief A pattern wrapping any callable returning something convertible to
     * bool.
     */
    template <typename F>
    struct predicate_pattern : base::pattern_base<predicate_pattern<F>> {
#if defined(__cpp_no_unique_address) && __cpp_no_unique_address >= 201803L
      [[no_unique_address]] F fn;
#else
      F fn;
#endif

      constexpr explicit predicate_pattern(F f) : fn(std::move(f)) {
      }

      /// @brief Match if `fn(x)` evaluates to true.
      template <typename X>
      constexpr bool match(X const &x) const
          noexcept(noexcept(std::declval<const F &>()(x))) {
        return fn(x);
      }

      /**
       * @brief Binding result for predicate patterns.
       */
      template <typename X>
      constexpr auto bind(const X & /*subj*/) const {
        return std::tuple<>{};
      }
    };

    /**
     * @brief Logical AND composition of two patterns.
     */
    template <typename L, typename R>
    struct and_pattern : base::pattern_base<and_pattern<L, R>> {
#if defined(__cpp_no_unique_address) && __cpp_no_unique_address >= 201803L
      [[no_unique_address]] L l;
      [[no_unique_address]] R r;
#else
      L l;
      R r;
#endif

      constexpr and_pattern(L lhs, R rhs)
          : l(std::move(lhs)), r(std::move(rhs)) {
      }

      template <typename X>
      constexpr bool match(X const &x) const
          noexcept(noexcept(l.match(x)) && noexcept(r.match(x))) {
        return l.match(x) && r.match(x);
      }

      /**
       * @brief Binding result for `and_pattern`.
       */
      template <typename X>
      constexpr auto bind(const X & /*subj*/) const {
        return std::tuple<>{};
      }
    };

    /**
     * @brief Logical OR composition of two patterns.
     */
    template <typename L, typename R>
    struct or_pattern : base::pattern_base<or_pattern<L, R>> {
#if defined(__cpp_no_unique_address) && __cpp_no_unique_address >= 201803L
      [[no_unique_address]] L l;
      [[no_unique_address]] R r;
#else
      L l;
      R r;
#endif

      constexpr or_pattern(L lhs, R rhs)
          : l(std::move(lhs)), r(std::move(rhs)) {
      }

      template <typename X>
      constexpr bool match(X const &x) const
          noexcept(noexcept(l.match(x)) && noexcept(r.match(x))) {
        return l.match(x) || r.match(x);
      }

      /**
       * @brief Binding result for `or_pattern`.
       */
      template <typename X>
      constexpr auto bind(const X & /*subj*/) const {
        return std::tuple<>{};
      }
    };

    /**
     * @brief Logical NOT composition of a pattern.
     */
    template <typename P>
    struct not_pattern : base::pattern_base<not_pattern<P>> {
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

      /**
       * @brief Binding result for `not_pattern`.
       */
      template <typename X>
      constexpr auto bind(const X & /*subj*/) const {
        return std::tuple<>{};
      }
    };

  } // namespace detail

  // --- Public API ---

  /**
   * @brief Factory for predicate_pattern.
   */
  template <typename F>
  constexpr auto pred(F &&f) {
    using Fn = std::decay_t<F>;
    return detail::predicate_pattern<Fn>(std::forward<F>(f));
  }

} // namespace ptn::pat::value

// --- Binding Contract Declarations ---

namespace ptn::pat::base {

  // --- for predicate_pattern ---
  template <typename F, typename Subject>
  struct binding_args<ptn::pat::value::detail::predicate_pattern<F>, Subject> {
    using tuple_type = std::tuple<>;
  };

  // --- for and_pattern ---
  template <typename L, typename R, typename Subject>
  struct binding_args<ptn::pat::value::detail::and_pattern<L, R>, Subject> {
    using tuple_type = std::tuple<>;
  };

  // --- for or_pattern ---
  template <typename L, typename R, typename Subject>
  struct binding_args<ptn::pat::value::detail::or_pattern<L, R>, Subject> {
    using tuple_type = std::tuple<>;
  };

  // --- for not_pattern ---
  template <typename P, typename Subject>
  struct binding_args<ptn::pat::value::detail::not_pattern<P>, Subject> {
    using tuple_type = std::tuple<>;
  };

} // namespace ptn::pat::base
