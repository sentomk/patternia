#pragma once
#include <type_traits>
#include <utility>
#include "ptn/config.hpp"

/**
 * @file pattern_tag.hpp
 * @brief Marker base type and detection utilities for Patternia patterns.
 *
 * This header defines the @ref ptn::detail::pattern_tag base struct and the
 * compile-time facilities for identifying "pattern-like" types.
 *
 * A type is considered *pattern-like* if:
 *   - It derives from `pattern_tag`, **or**
 *   - It provides a valid `match(subject)` member returning something
 *     convertible to `bool`.
 *
 * This enables Patternia's DSL (`match().when(Pattern >> Handler)`) to accept
 * both built-in and user-defined patterns with consistent compile-time rules.
 *
 * @ingroup patterns
 */

namespace ptn::detail {
  /**
   * @brief C++17 fallback: checks whether `P::match(x)` is valid.
   *
   * This trait detects whether `const P&` can be invoked with
   * `p.match(subj)` and the result is convertible to `bool`.
   *
   * It is used when `PTN_USE_CONCEPTS` is disabled.
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

  /**
   * @brief Marker base type for all built-in Patternia patterns.
   *
   * Any pattern in Patternia should inherit from `pattern_tag` to signal
   * that it participates in the pattern-matching DSL. This enables
   * uniform compile-time detection and improves diagnostic quality.
   *
   * Example:
   * @code{.cpp}
   * struct value_pattern : pattern_tag {
   *     bool match(int x) const noexcept { return x == 42; }
   * };
   * @endcode
   */
  struct pattern_tag {
    static constexpr bool is_pattern = true;
  };

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
  concept pattern_like =
      std::derived_from<P, pattern_tag> || requires(const P &p, auto &&subj) {
        { p.match(subj) } -> std::convertible_to<bool>;
      };

#else // C++17 fallback

  /**
   * @brief C++17-compatible trait checking whether a type behaves like a
   * Pattern.
   *
   * A type `P` is pattern-like if:
   *   - `P` derives from pattern_tag, OR
   *   - `P` has a member `match(subj)` returning something `bool`-convertible.
   *
   * Patternia uses this trait to constrain `.when(...)` overloads on C++17
   * mode.
   *
   * @tparam P The type being tested.
   */
  template <typename P>
  struct is_pattern_like : std::integral_constant<
                               bool,
                               std::is_base_of_v<pattern_tag, P> ||
                                   ptn::detail::has_match_method<P>::value> {};
#endif

} // namespace ptn::detail
