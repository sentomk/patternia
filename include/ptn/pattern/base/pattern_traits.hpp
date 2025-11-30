#pragma once
#include <type_traits>
#include <utility>
#include "ptn/config.hpp"

/**
 * @file pattern_traits.hpp
 * @brief Compile-time detection utilities for Patternia patterns.
 */

namespace ptn::pat::detail {
  /** Pattern identification base */

  template <typename P, typename = void>
  struct has_match_method : std::false_type {};

  template <typename P>
  struct has_match_method<
      P,
      std::void_t<decltype(static_cast<bool>(std::declval<const P &>().match(
          std::declval<int>())))>> : std::true_type {};

  /** Pattern identification base */

  /**
   * @brief Marker base type for all built-in Patternia patterns.
   */
  struct pattern_tag {
    static constexpr bool is_pattern = true;
  };

  /** Pattern identification base */
#if defined(PTN_USE_CONCEPTS) && PTN_USE_CONCEPTS

  /**
   * @brief C++20 Concept: determines whether a type behaves like a Pattern.
   * @tparam P The type being tested.
   */
  template <typename P>
  concept pattern =
      std::derived_from<P, pattern_tag> || requires(const P &p, auto &&subj) {
        { p.match(subj) } -> std::convertible_to<bool>;
      };

#else // C++17 fallback

  /**
   * @brief Trait: determines whether P acts as a Pattern.
   */
  template <typename P>
  struct is_pattern : std::integral_constant<
                          bool,
                          std::is_base_of_v<pattern_tag, P> ||
                              ptn::pat::detail::has_match_method<P>::value> {};

  /**
   * @brief Convenience variable template for is_pattern<P>::value.
   */
  template <typename P>
  inline constexpr bool is_pattern_v = is_pattern<P>::value;

#endif

} // namespace ptn::pat::detail
