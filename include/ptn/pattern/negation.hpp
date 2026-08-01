#pragma once

#include <type_traits>
#include <tuple>
#include <utility>

#include "ptn/pattern/base/fwd.h"
#include "ptn/pattern/base/pattern_base.hpp"
#include "ptn/pattern/base/pattern_traits.hpp"

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

  // Operator sugar: `!p` is equivalent to `neg(p)`. Only pattern
  // objects participate; guard predicates are excluded so this
  // never collides with boolean logic over predicates.
  //
  // Declared in ptn::pat::base so ADL finds it for every pattern:
  // all patterns derive from base::pattern_base, which makes this
  // namespace associated even though the concrete pattern types
  // live in ptn::pat::detail (ADL does not ascend namespaces).
  template <
      typename P,
      std::enable_if_t<
          std::is_base_of_v<pattern_tag, std::decay_t<P>>
              && !pat::traits::is_guard_predicate_v<std::decay_t<P>>,
          int> = 0>
  constexpr auto operator!(P &&p) {
    return pat::neg(std::forward<P>(p));
  }

} // namespace ptn::pat::base
