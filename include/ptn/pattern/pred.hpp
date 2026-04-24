#pragma once

// Public API and implementation for predicate patterns
// (`pred(callable)`).
//
// This file provides a factory function to create patterns that
// match against a subject by applying an arbitrary unary predicate.
// The predicate is evaluated during the match phase rather than as a
// post-bind guard.
//
// Usage:
//   match(x) | on(
//     pred([](int v) { return v % 2 == 0; }) >> "even",
//     _ >> "odd"
//   );

#include <tuple>
#include <type_traits>
#include <utility>

#include "ptn/pattern/base/fwd.h"
#include "ptn/pattern/base/pattern_base.hpp"

namespace ptn::pat {

  namespace detail {

    // A pattern that matches a subject by invoking a stored unary
    // predicate.
    //
    // The predicate must be callable with the subject value and
    // return a type convertible to bool. No values are bound by this
    // pattern.
    template <typename Callable>
    struct pred_pattern
        : base::pattern_base<pred_pattern<Callable>> {
      Callable fn;

      template <typename F,
                typename = std::enable_if_t<
                    !std::is_same_v<std::decay_t<F>, pred_pattern>>>
      constexpr explicit pred_pattern(F &&f)
          : fn(std::forward<F>(f)) {
      }

      // Matches when the stored predicate returns true for the
      // subject.
      template <typename Subject>
      constexpr bool match(Subject &&subj) const noexcept(noexcept(
          static_cast<bool>(fn(std::forward<Subject>(subj))))) {
        return static_cast<bool>(fn(std::forward<Subject>(subj)));
      }

      // Binds no values.
      template <typename Subject>
      constexpr auto bind(const Subject & /*subj*/) const noexcept {
        return std::tuple<>{};
      }
    };

  } // namespace detail

  // Returns a pattern that succeeds when the callable returns true
  // for the matched subject.
  //
  // The callable must be a unary function (or function object)
  // accepting the subject type and returning a value convertible to
  // bool.
  template <typename Callable>
  constexpr auto pred(Callable &&fn) {
    return detail::pred_pattern<std::decay_t<Callable>>(
        std::forward<Callable>(fn));
  }

} // namespace ptn::pat

// --- Binding Contract Declaration ---

namespace ptn::pat::base {

  template <typename Callable, typename Subject>
  struct binding_args<ptn::pat::detail::pred_pattern<Callable>,
                      Subject> {
    using type = std::tuple<>;
  };

} // namespace ptn::pat::base
