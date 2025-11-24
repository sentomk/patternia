#pragma once
/**
 * @file type_pattern.hpp (detail)
 * @brief Internal implementation of type-based patterns for Patternia.
 *
 * This is an internal header. Users should use the public factory functions
 * from `ptn/pattern/type/type.hpp` instead of directly instantiating these
 * types.
 *
 * Part of the Pattern Layer (namespace ptn::pattern::type::detail)
 */

#include <type_traits>
#include <utility>

#include "ptn/pattern/detail/pattern_base.hpp"
#include "ptn/meta/meta.hpp" // meta utilities

namespace ptn::pattern::type::detail {

  using namespace ptn::meta;

  //====================================================================
  //  type::is<T>  — static type equality
  //====================================================================

  template <typename T>
  struct is_pattern : ptn::pattern::detail::pattern_base<is_pattern<T>> {

    template <typename X>
    constexpr bool match(X &&) const noexcept {
      using D = remove_cvref_t<X>;
      return std::is_same_v<D, T>;
    }

    template <typename X>
    constexpr auto bind(X &&x) const noexcept {
      return std::forward<X>(x);
    }
  };

  //====================================================================
  //  type::in<Ts...>  — membership
  //====================================================================

  template <typename... Ts>
  struct in_pattern : ptn::pattern::detail::pattern_base<in_pattern<Ts...>> {

    template <typename X>
    constexpr bool match(X &&) const noexcept {
      using D    = remove_cvref_t<X>;
      using list = type_list<Ts...>;
      return contains_v<D, list>;
    }

    template <typename X>
    constexpr auto bind(X &&x) const noexcept {
      return std::forward<X>(x);
    }
  };

  //====================================================================
  //  type::not_in<Ts...>
  //====================================================================

  template <typename... Ts>
  struct not_in_pattern
      : ptn::pattern::detail::pattern_base<not_in_pattern<Ts...>> {

    template <typename X>
    constexpr bool match(X &&x) const noexcept {
      using D    = remove_cvref_t<X>;
      using list = type_list<Ts...>;
      return !contains_v<D, list>;
    }

    template <typename X>
    constexpr auto bind(X &&x) const noexcept {
      return std::forward<X>(x);
    }
  };

  //====================================================================
  //  type::from<Tpl>  — match template family
  //====================================================================

  template <template <typename...> typename Tpl>
  struct from_pattern : ptn::pattern::detail::pattern_base<from_pattern<Tpl>> {

    template <typename X>
    constexpr bool match(X &&) const noexcept {
      using D = remove_cvref_t<X>;
      return is_spec_of_v<Tpl, D>;
    }

    template <typename X>
    constexpr auto bind(X &&x) const noexcept {
      return std::forward<X>(x);
    }
  };

} // namespace ptn::pattern::type::detail
