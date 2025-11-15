#pragma once
/**
 * @file type.hpp
 * @brief Static type-based patterns for Patternia:
 *
 *   - type::is<T>        : exact type match
 *   - type::in<Ts...>    : membership in a type-set
 *   - type::not_in<Ts...>: negated membership
 *   - type::from<Tpl>    : any specialization of template Tpl<...>
 */

#include <type_traits>
#include <utility>

#include "ptn/pattern/pattern_base.hpp"
#include "ptn/meta/meta.hpp" // meta utilities

namespace ptn::pattern::type {

  using namespace ptn::meta;

  //====================================================================
  //  type::is<T>  — static type equality
  //====================================================================

  template <typename T>
  struct is_pattern : ptn::pattern::pattern_base<is_pattern<T>> {

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

  template <typename T>
  inline constexpr is_pattern<T> is{};

  //====================================================================
  //  type::in<Ts...>  — membership
  //====================================================================

  template <typename... Ts>
  struct in_pattern : ptn::pattern::pattern_base<in_pattern<Ts...>> {

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

  template <typename... Ts>
  inline constexpr in_pattern<Ts...> in{};

  //====================================================================
  //  type::not_in<Ts...>
  //====================================================================

  template <typename... Ts>
  struct not_in_pattern : ptn::pattern::pattern_base<not_in_pattern<Ts...>> {

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

  template <typename... Ts>
  inline constexpr not_in_pattern<Ts...> not_in{};

  //====================================================================
  //  type::from<Tpl>  — match template family
  //====================================================================

  template <template <typename...> typename Tpl>
  struct from_pattern : ptn::pattern::pattern_base<from_pattern<Tpl>> {

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

  template <template <typename...> typename Tpl>
  inline constexpr from_pattern<Tpl> from{};

  //====================================================================
  //  Legacy compatibility
  //====================================================================

  template <typename T>
  using type_tag = is_pattern<T>;

} // namespace ptn::pattern::type
