#pragma once
/**
 * @file type.hpp
 * @brief Public API for type-based patterns in Patternia:
 *
 *   - type::is<T>        : exact type match
 *   - type::in<Ts...>    : membership in a type-set
 *   - type::not_in<Ts...>: negated membership
 *   - type::from<Tpl>    : any specialization of template Tpl<...>
 *
 * This module provides the public factory functions for creating type patterns.
 * The actual implementation types are in the `detail/` subdirectory and should
 * not be used directly by end users.
 *
 * Part of the Pattern Layer (namespace ptn::pattern::type)
 */

#include "ptn/pattern/type/detail/type_pattern.hpp"

namespace ptn::pattern::type {

  //====================================================================
  //  type::is<T>  — static type equality
  //====================================================================

  /**
   * @brief Pattern that matches subjects of exactly type T.
   *
   * @tparam T The exact type to match.
   * @return A type pattern that matches subjects of type T.
   *
   * @example
   * ```cpp
   * auto result = ptn::match(value)
   *     .when(type::is<int> >> "integer")
   *     .when(type::is<double> >> "double")
   *     .otherwise("other");
   * ```
   */
  template <typename T>
  inline constexpr detail::is_pattern<T> is{};

  //====================================================================
  //  type::in<Ts...>  — membership
  //====================================================================

  /**
   * @brief Pattern that matches subjects whose type is in the set {Ts...}.
   *
   * @tparam Ts... The set of types to match against.
   * @return A type pattern that matches subjects of any type in Ts.
   *
   * @example
   * ```cpp
   * auto result = ptn::match(value)
   *     .when(type::in<int, float, double> >> "numeric")
   *     .otherwise("non-numeric");
   * ```
   */
  template <typename... Ts>
  inline constexpr detail::in_pattern<Ts...> in{};

  //====================================================================
  //  type::not_in<Ts...>  — negated membership
  //====================================================================

  /**
   * @brief Pattern that matches subjects whose type is NOT in the set {Ts...}.
   *
   * @tparam Ts... The set of types to exclude.
   * @return A type pattern that matches subjects of any type not in Ts.
   *
   * @example
   * ```cpp
   * auto result = ptn::match(value)
   *     .when(type::not_in<int, float> >> "not int or float")
   *     .otherwise("int or float");
   * ```
   */
  template <typename... Ts>
  inline constexpr detail::not_in_pattern<Ts...> not_in{};

  //====================================================================
  //  type::from<Tpl>  — match template family
  //====================================================================

  /**
   * @brief Pattern that matches subjects that are specializations of template Tpl.
   *
   * @tparam Tpl The template to match against.
   * @return A type pattern that matches any specialization of Tpl.
   *
   * @example
   * ```cpp
   * auto result = ptn::match(container)
   *     .when(type::from<std::vector> >> "vector")
   *     .when(type::from<std::list> >> "list")
   *     .otherwise("other container");
   * ```
   */
  template <template <typename...> typename Tpl>
  inline constexpr detail::from_pattern<Tpl> from{};

  //====================================================================
  //  Legacy compatibility
  //====================================================================

  /**
   * @brief Legacy alias for type::is<T>.
   *
   * @deprecated Use type::is<T> instead.
   */
  template <typename T>
  using type_tag = detail::is_pattern<T>;

} // namespace ptn::pattern::type
