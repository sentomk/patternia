#pragma once

#include <type_traits>
#include <tuple>
#include <utility>

#include "ptn/pattern/base/fwd.h"

namespace ptn::pat {

  namespace detail {
    template <typename P>
    struct negation_pattern
        : base::pattern_base<negation_pattern<P>> {
      P sub;

      constexpr explicit negation_pattern(P p) : sub(std::move(p)) {
      }

      template <typename X>
      constexpr bool match(X const &x) const
          noexcept(noexcept(!sub.match(x))) {
        return !sub.match(x);
      }

      template <typename X>
      constexpr auto bind(const X &subj) const {
        return std::tuple<>{};
      }
    };
  } // namespace detail

  template <typename P>
  constexpr auto neg(P &&p) {
    return detail::negation_pattern<std::decay_t<P>>(
        std::forward<P>(p));
  }

} // namespace ptn::pat

namespace ptn::pat::base {

  template <typename P, typename Subject>
  struct binding_args<ptn::pat::detail::negation_pattern<P>,
                      Subject> {
    using type = std::tuple<>;
  };

} // namespace ptn::pat::base
