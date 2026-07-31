#pragma once

// Public API and implementation for wildcard patterns (`_`).
//
// This file provides the wildcard pattern that matches any value
// without binding, plus the expression hooks used by guard
// operators. Include modifiers/guard.hpp (or patternia.hpp) to use
// `_` in a guard expression such as `_ > 0`.

#include "ptn/pattern/base/fwd.h"
#include "ptn/pattern/base/pattern_base.hpp"
#include "ptn/pattern/base/pattern_traits.hpp"
#include "ptn/pattern/modifiers/placeholder.hpp"

#include <tuple>
#include <utility>

namespace ptn::pat {
  namespace detail {
    // Wildcard pattern that matches any value and binds nothing.
    struct wildcard_t : base::pattern_base<wildcard_t>,
                        mod::arg_t<0> {

      // Always matches successfully.
      template <typename Subject>
      constexpr bool match(Subject &&) const noexcept {
        return true;
      }

      // Binds no values.
      template <typename Subject>
      constexpr auto bind(const Subject &) const {
        return std::tuple<>{};
      }
    };
  } // namespace detail

  // Global wildcard and, when guard operators are included,
  // single-value guard placeholder.
  inline constexpr ptn::pat::detail::wildcard_t _{};

} // namespace ptn::pat

namespace ptn::pat::traits {

  template <>
  struct is_arg_expr<ptn::pat::detail::wildcard_t> : std::true_type {
  };

} // namespace ptn::pat::traits

namespace ptn::pat::mod {

  template <>
  struct max_arg_index<ptn::pat::detail::wildcard_t>
      : std::integral_constant<std::size_t, 0> {};

  template <typename Tuple>
  constexpr decltype(auto) eval(const ptn::pat::detail::wildcard_t &,
                                Tuple &&tuple) {
    return std::get<0>(std::forward<Tuple>(tuple));
  }

} // namespace ptn::pat::mod

namespace ptn::pat::base {

  // Wildcard pattern binds no values.
  template <typename Subject>
  struct binding_args<ptn::pat::detail::wildcard_t, Subject> {
    using type = std::tuple<>;
  };
} // namespace ptn::pat::base
