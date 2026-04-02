#pragma once

// Public API and implementation for pattern combinators (`any(...)`,
// `all(...)`).
//
// This file provides non-binding composition patterns that combine
// multiple sub-patterns with OR/AND semantics.

#include <tuple>
#include <type_traits>
#include <utility>

#include "ptn/pattern/base/fwd.h"
#include "ptn/pattern/base/pattern_base.hpp"

namespace ptn::pat {

  namespace detail {

    // Matches when any sub-pattern matches. This combinator never
    // contributes bindings; it is used only for control flow.
    template <typename... Patterns>
    struct any_pattern : base::pattern_base<any_pattern<Patterns...>> {
      std::tuple<Patterns...> patterns;

      template <typename... Ps,
                typename = std::enable_if_t<sizeof...(Ps) == sizeof...(Patterns)>>
      constexpr explicit any_pattern(Ps &&...ps)
          : patterns(std::forward<Ps>(ps)...) {
      }

      template <typename Subject>
      constexpr bool match(const Subject &subject) const {
        return std::apply(
            [&](const auto &...ps) {
              // Fold over || to preserve left-to-right short-circuit
              // evaluation.
              return (ps.match(subject) || ...);
            },
            patterns);
      }

      template <typename Subject>
      constexpr auto bind(const Subject & /*subject*/) const {
        return std::tuple<>{};
      }
    };

    // Matches only when every sub-pattern matches. Like any_pattern,
    // this combinator is non-binding.
    template <typename... Patterns>
    struct all_pattern : base::pattern_base<all_pattern<Patterns...>> {
      std::tuple<Patterns...> patterns;

      template <typename... Ps,
                typename = std::enable_if_t<sizeof...(Ps) == sizeof...(Patterns)>>
      constexpr explicit all_pattern(Ps &&...ps)
          : patterns(std::forward<Ps>(ps)...) {
      }

      template <typename Subject>
      constexpr bool match(const Subject &subject) const {
        return std::apply(
            [&](const auto &...ps) {
              // Fold over && to preserve left-to-right short-circuit
              // evaluation.
              return (ps.match(subject) && ...);
            },
            patterns);
      }

      template <typename Subject>
      constexpr auto bind(const Subject & /*subject*/) const {
        return std::tuple<>{};
      }
    };

  } // namespace detail

  // Returns a pattern that succeeds on the first matching
  // sub-pattern.
  template <typename... Patterns>
  constexpr auto any(Patterns &&...patterns) {
    static_assert(
        sizeof...(Patterns) > 0,
        "[Patternia.any]: requires at least one sub-pattern.");
    static_assert(
        (std::is_base_of_v<base::pattern_tag, std::decay_t<Patterns>> && ...),
        "[Patternia.any]: every argument must be a pattern object.");

    return detail::any_pattern<std::decay_t<Patterns>...>(
        std::forward<Patterns>(patterns)...);
  }

  // Returns a pattern that succeeds only when every sub-pattern
  // matches.
  template <typename... Patterns>
  constexpr auto all(Patterns &&...patterns) {
    static_assert(
        sizeof...(Patterns) > 0,
        "[Patternia.all]: requires at least one sub-pattern.");
    static_assert(
        (std::is_base_of_v<base::pattern_tag, std::decay_t<Patterns>> && ...),
        "[Patternia.all]: every argument must be a pattern object.");

    return detail::all_pattern<std::decay_t<Patterns>...>(
        std::forward<Patterns>(patterns)...);
  }

} // namespace ptn::pat

namespace ptn::pat::base {

  template <typename... Patterns, typename Subject>
  struct binding_args<ptn::pat::detail::any_pattern<Patterns...>, Subject> {
    using type = std::tuple<>;
  };

  template <typename... Patterns, typename Subject>
  struct binding_args<ptn::pat::detail::all_pattern<Patterns...>, Subject> {
    using type = std::tuple<>;
  };

} // namespace ptn::pat::base
