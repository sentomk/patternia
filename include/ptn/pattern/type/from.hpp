#pragma once
/**
 * @file from.hpp
 * @brief Implements `ptn::pattern::type::from<Tpl>`.
 */

#include <utility>
#include <tuple>
#include "ptn/pattern/base/fwd.h"
#include "ptn/meta/meta.hpp"

namespace ptn::pat::type::detail {

  template <template <typename...> typename Tpl>
  struct from_pattern : ptn::pat::base::pattern_base<from_pattern<Tpl>> {

    template <typename X>
    constexpr bool match(X &&) const noexcept {
      using D = ptn::meta::remove_cvref_t<X>;
      return ptn::meta::is_spec_of_v<Tpl, D>;
    }

    template <typename X>
    constexpr auto bind(X &&x) const noexcept {
      return std::forward_as_tuple(std::forward<X>(x));
    }
  };

} // namespace ptn::pat::type::detail

// --- Public API ---
namespace ptn::pat::type {
  template <template <typename...> typename Tpl>
  inline constexpr detail::from_pattern<Tpl> from{};
} // namespace ptn::pat::type

// --- Binding Contract ---
namespace ptn::pat::base {
  template <template <typename...> typename Tpl, typename Subject>
  struct binding_args<ptn::pat::type::detail::from_pattern<Tpl>, Subject> {
    // from_pattern 将匹配到的值绑定给处理器
    using type = std::tuple<Subject>;
  };
} // namespace ptn::pat::base
