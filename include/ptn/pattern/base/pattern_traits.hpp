#pragma once
#include <type_traits>
#include <utility>

// Compile-time detection utilities for Patternia patterns.

namespace ptn::pat::detail {
  // Pattern identification base

  template <typename P, typename = void>
  struct has_match_method : std::false_type {};

  template <typename P>
  struct has_match_method<
      P,
      std::void_t<decltype(static_cast<bool>(std::declval<const P &>().match(
          std::declval<int>())))>> : std::true_type {};

  // Pattern identification base

  // Marker base type for all built-in Patternia patterns.
  struct pattern_tag {
    static constexpr bool is_pattern = true;
  };

  // Pattern identification base
  // Trait: determines whether P acts as a Pattern.
  template <typename P>
  struct is_pattern : std::integral_constant<
                          bool,
                          std::is_base_of_v<pattern_tag, P> ||
                              ptn::pat::detail::has_match_method<P>::value> {};

  // Convenience variable template for is_pattern<P>::value.
  template <typename P>
  inline constexpr bool is_pattern_v = is_pattern<P>::value;

} // namespace ptn::pat::detail
