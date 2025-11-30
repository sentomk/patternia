#pragma once
/**
 * @file not_in.hpp
 * @brief Implements `ptn::pattern::type::not_in<Ts...>`.
 */

#include <type_traits>
#include <utility>
#include <tuple>
#include "ptn/pattern/base/fwd.h"
#include "ptn/meta/meta.hpp"

namespace ptn::pat::type::detail {

  template <typename... Ts>
  struct not_in_pattern : ptn::pat::base::pattern_base<not_in_pattern<Ts...>> {
    template <typename X>
    constexpr bool match(X &&) const noexcept {
      using D    = ptn::meta::remove_cvref_t<X>;
      using list = ptn::meta::type_list<Ts...>;
      return !ptn::meta::contains_v<D, list>;
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
  struct binding_args<ptn::pat::type::detail::not_in_pattern<Ts...>, Subject> {
    // not_in_pattern 将匹配到的值绑定给处理器
    using type = std::tuple<Subject>;
  };
} // namespace ptn::pat::base

// --- Public API ---
namespace ptn::pat::type {
  template <typename... Ts>
  inline constexpr detail::not_in_pattern<Ts...> not_in{};
} // namespace ptn::pat::type
