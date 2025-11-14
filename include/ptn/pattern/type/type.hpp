#pragma once
/**
 * @file type_tag.hpp
 * @brief Defines `type_tag<T>` / `type_c<T>` as both a type tag and
 *        a Patternia pattern (matches by static type).
 *
 * Part of Pattern Layer (namespace ptn::pattern::value)
 */

#include <type_traits>
#include "ptn/pattern/pattern_base.hpp"

namespace ptn::pattern::type {

  /**
   * @brief Compile-time wrapper around a C++ type, and also a pattern
   *        that matches when the subject's (decayed) type equals `T`.
   */
  template <typename T>
  struct type_tag : ptn::pattern::pattern_base<type_tag<T>> {

    template <typename X>
    constexpr bool match(X &&) const noexcept {
      return std::is_same_v<T, std::decay_t<X>>;
    }

    template <typename X>
    constexpr auto bind(X &&x) const noexcept {
      return std::forward<X>(x);
    }
  };

  /**
   * @brief Shorthand constant for `type_tag<T>`.
   */
  template <typename T>
  inline constexpr type_tag<T> type_is{};

} // namespace ptn::pattern::type
