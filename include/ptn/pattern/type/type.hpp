#pragma once
/**
 * @file type.hpp
 * @brief Static type-based patterns for Patternia.
 *
 * Provides the static Type Pattern DSL:
 *   - type::is<T>        : match when subject's type is exactly T
 *   - type::in<Ts...>    : match when subject's type is in Ts...
 *   - type::not_in<Ts...>
 *   - type::from<Tpl>    : match any specialization of template Tpl<...>
 *
 * Legacy:
 *   - type_is<T> alias kept as compatibility wrapper → type::is<T>
 *
 * This file forms the Pattern Layer entry for type patterns.
 */

#include <type_traits>
#include <utility>

#include "ptn/pattern/pattern_base.hpp"

namespace ptn::pattern::type {

  //====================================================================
  //  Internal helper: is specialization of template?
  //====================================================================
  namespace detail {

    template <template <typename...> typename Tpl, typename T>
    struct is_specialization_of : std::false_type {};

    template <template <typename...> typename Tpl, typename... Args>
    struct is_specialization_of<Tpl, Tpl<Args...>> : std::true_type {};

  } // namespace detail

  //====================================================================
  //  type::is<T>  — static type equality
  //====================================================================

  template <typename T>
  struct is_pattern : ptn::pattern::pattern_base<is_pattern<T>> {

    template <typename X>
    constexpr bool match(X &&) const noexcept {
      return std::is_same_v<std::decay_t<X>, T>;
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
      using D = std::decay_t<X>;
      return (std::is_same_v<D, Ts> || ...);
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
    constexpr bool match(X &&v) const noexcept {
      return !in_pattern<Ts...>{}.match(std::forward<X>(v));
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
      using D = std::decay_t<X>;
      return detail::is_specialization_of<Tpl, D>::value;
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

  template <typename T>
  inline constexpr auto type_is = is<T>;

} // namespace ptn::pattern::type
