#pragma once
#include <type_traits>
#include <utility>
#include "ptn/config.hpp"

/**
 * @file pattern_traits.hpp
 * @brief Compile-time detection utilities for Patternia patterns.
 *
 * This header defines:
 *  - pattern_tag   : marker base class
 *  - has_match_method<P>
 *  - concept pattern (C++20)
 *  - trait is_pattern<P> / is_pattern_v (C++17 fallback)
 *
 * All type-detection logic for the Pattern Layer is centralized here.
 *
 * namespace ptn::pattern:detail
 */

namespace ptn::pattern::detail {
  // ---------------------------------------------------------------------------
  //  has_match_method  (C++17 detection for user-defined patterns)
  // ---------------------------------------------------------------------------

  /**
   * @brief C++17 SFINAE: checks whether `P::match(subj)` is a valid expression
   *        convertible to bool.
   *
   * Used in environments where concepts are disabled (`PTN_USE_CONCEPTS=0`).
   *
   * @tparam P Type being inspected.
   */
  template <typename P, typename = void>
  struct has_match_method : std::false_type {};

  template <typename P>
  struct has_match_method<
      P,
      std::void_t<decltype(static_cast<bool>(std::declval<const P &>().match(
          std::declval<int>())))>> : std::true_type {};

  // ---------------------------------------------------------------------------
  //  pattern_tag  (marker base type)
  // ---------------------------------------------------------------------------

  /**
   * @brief Marker base type for all built-in Patternia patterns.
   *
   * Deriving from this type signals that a type participates in Patternia's
   * DSL.
   *
   */
  struct pattern_tag {
    static constexpr bool is_pattern = true;
  };

  // ---------------------------------------------------------------------------
  //  is_pattern
  // ---------------------------------------------------------------------------
//  Concepts-based detection (C++20)
#if defined(PTN_USE_CONCEPTS) && PTN_USE_CONCEPTS

  /**
   * @brief C++20 Concept: determines whether a type behaves like a Pattern.
   *
   * A type P is considered pattern-like if:
   *   - it derives from `pattern_tag`, OR
   *   - it defines a valid `match(x)` operation returning something
   *     convertible to `bool`.
   *
   * This makes Patternia extensible: user-defined types with
   * `match(subject)` automatically qualify as patterns without needing
   * inheritance from `pattern_tag`.
   *
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
  struct is_pattern
      : std::integral_constant<
            bool,
            std::is_base_of_v<pattern_tag, P> ||
                ptn::pattern::detail::has_match_method<P>::value> {};

  /**
   * @brief Convenience variable template for is_pattern<P>::value.
   */
  template <typename P>
  inline constexpr bool is_pattern_v = is_pattern<P>::value;

#endif

} // namespace ptn::pattern::detail
