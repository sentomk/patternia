#pragma once

// Forward declarations for Patternia Core Layer.
//
// This header contains forward declarations to avoid circular dependencies
// and provide minimal interface information for the core components.

namespace ptn {

  // Public API Functions

  /// Primary match function for single-target pattern matching
  template <typename T>
  constexpr auto match(T &&);

  /// Match function with explicit result type specification
  template <typename U, typename T>
  constexpr auto match(T &&);

  namespace core {
    /// Public-facing match builder class for fluent pattern matching
    template <typename TV, typename... Cases>
    class match_builder;
  }

  // Core Internal Components

  namespace core::engine::detail {
    /// Internal implementation of match builder
    template <typename TV, typename... Cases>
    class match_builder;
  }

  namespace core::dsl::detail {
    /// Internal case expression structure for DSL operations
    template <typename Pattern, typename Handler>
    struct case_expr;
  }

  // Common Traits (Forward Declarations)

  namespace core::common {

    /// Type trait to detect if a type satisfies pattern requirements
    /// C++17 SFINAE fallback for `pattern_like` concept
    template <typename P, typename = void>
    struct is_pattern;

    /// Convenience variable template for is_pattern trait
    template <typename P>
    inline constexpr bool is_pattern_v = is_pattern<P>::value;

    /// Core binding trait for extracting bound variables from patterns
    /// Primary template - specializations should be provided in pattern headers
    template <typename Pattern, typename Subject>
    struct binding_args;

  } // namespace core::common

} // namespace ptn
