#pragma once

// Implements `ptn::pattern::type::in<Ts...>`.

#include <type_traits>
#include <utility>
#include <tuple>
#include "ptn/pattern/base/fwd.h"
#include "ptn/meta/meta.hpp"

namespace ptn::pat::type::detail {

  template <typename... Ts>
  struct in_pattern : ptn::pat::base::pattern_base<in_pattern<Ts...>> {
    template <typename X>
    constexpr bool match(X &&) const noexcept {
      using D    = ptn::meta::remove_cvref_t<X>;
      using list = ptn::meta::type_list<Ts...>;
      return ptn::meta::contains_v<D, list>;
    }

    template <typename X>
    constexpr auto bind(X &&x) const noexcept {
      return std::forward_as_tuple(std::forward<X>(x));
    }
  };

} // namespace ptn::pat::type::detail

// --- Binding Contract ---
namespace ptn::pat::base {
  template <typename... Ts, typename Subject>
  struct binding_args<ptn::pat::type::detail::in_pattern<Ts...>, Subject> {
    // in_pattern binds the matched value to the handler
    using type = std::tuple<Subject>;
  };
} // namespace ptn::pat::base

// --- Public API ---
namespace ptn::pat::type {
  template <typename... Ts>
  inline constexpr detail::in_pattern<Ts...> in{};
} // namespace ptn::pat::type
