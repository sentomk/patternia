#pragma once
/**
 * @file is.hpp
 * @brief Implements `ptn::pattern::type::is<T>`.
 */

#include <type_traits>
#include <utility>
#include <tuple>
#include "ptn/pattern/base/fwd.h"
#include "ptn/meta/meta.hpp"

namespace ptn::pat::type::detail {

  template <typename T>
  struct is_pattern : ptn::pat::base::pattern_base<is_pattern<T>> {
    template <typename X>
    constexpr bool match(X &&) const noexcept {
      using D = ptn::meta::remove_cvref_t<X>;
      return std::is_same_v<D, T>;
    }

    template <typename X>
    constexpr auto bind(X &&x) const noexcept {
      return std::forward_as_tuple(std::forward<X>(x));
    }
  };

} // namespace ptn::pat::type::detail

// --- Binding Contract ---
namespace ptn::pat::base {
  template <typename T, typename Subject>
  struct binding_args<ptn::pat::type::detail::is_pattern<T>, Subject> {
    // is_pattern 将匹配到的值绑定给处理器
    using type = std::tuple<Subject>;
  };
} // namespace ptn::pat::base

// --- Public API ---
namespace ptn::pat::type {
  template <typename T>
  inline constexpr detail::is_pattern<T> is{};
} // namespace ptn::pat::type
