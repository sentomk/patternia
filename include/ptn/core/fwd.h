#pragma once

// Forward declarations for Patternia Core Layer.

namespace ptn {

  // Public API

  template <typename T>
  constexpr auto match(T &&);

  template <typename U, typename T>
  constexpr auto match(T &&);

  namespace core {
    template <typename TV, typename... Cases>
    class match_builder; // Public-facing alias
  }

  // Core Internals

  namespace core::engine::detail {
    template <typename TV, typename... Cases>
    class match_builder; // Internal implementation
  }

  namespace core::dsl::detail {
    template <typename Pattern, typename Handler>
    struct case_expr;
  }

  // Common Traits (Forward Declarations)

  namespace core::common {

    // C++17 SFINAE fallback for `pattern_like`.
    template <typename P, typename = void>
    struct is_pattern;

    template <typename P>
    inline constexpr bool is_pattern_v = is_pattern<P>::value;

    // Forward declaration of the core binding trait.
    //
    // This is the primary template. Specializations for concrete patterns
    // should be provided in the same header as the pattern definition.
    template <typename Pattern, typename Subject>
    struct binding_args;

  } // namespace core::common

} // namespace ptn
