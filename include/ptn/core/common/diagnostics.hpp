#pragma once

// Compile-time diagnostics and static assertions for Patternia.
//
// This header provides validation utilities that detect common pattern matching
// errors at compile time, providing clear error messages to guide developers.

#include <type_traits>
#include "ptn/core/common/common_traits.hpp"

namespace ptn::core::common {

  // Validates the entire match expression for consistency
  // Ensures all handlers are invocable and have compatible return types
  template <typename Subject, typename Otherwise, typename... Cases>
  constexpr void static_assert_valid_match() {
    // Verify all case handlers can be invoked with their pattern's bound
    // arguments
    static_assert(
        (traits::is_handler_invocable_v<Cases, Subject> && ...),
        "[Patternia Error] At least one case's handler cannot be invoked with "
        "the arguments bound by its pattern. "
        "Please check the handler's signature against the pattern's expected "
        "bindings.");

    // Verify the otherwise handler has a valid signature
    static_assert(
        std::is_invocable_v<Otherwise, Subject> ||
            std::is_invocable_v<Otherwise>,
        "[Patternia Error] The `otherwise` handler has an invalid signature. "
        "It should be either callable with the subject value or callable with "
        "no arguments.");

    // Verify all handlers have a common return type
    using common_return_type =
        traits::match_result_t<Subject, Otherwise, Cases...>;

    // Force instantiation to catch potential type errors (skip for void)
    if constexpr (!std::is_void_v<common_return_type>) {
      (void) sizeof(common_return_type);
    }
  }

  // Validates that a handler signature matches a pattern's binding requirements
  template <typename Case, typename Subject>
  constexpr void static_assert_valid_handler() {
    static_assert(
        traits::is_handler_invocable_v<Case, Subject>,
        "[Patternia Error] Handler signature does not match the pattern's "
        "binding result.");
  }

  // Validates a single case expression structure and handler compatibility
  template <typename Case, typename Subject>
  constexpr void static_assert_valid_case() {
    static_assert(
        traits::is_case_expr_v<Case>,
        "[Patternia Error] Argument to `.when()` must be a case expression "
        "created with the '>>' operator.");
    static_assert_valid_handler<Case, Subject>();
  }

  // Validates that a type satisfies the pattern requirements
  template <typename Pattern, typename Subject>
  constexpr void static_assert_valid_pattern() {
    static_assert(
        ptn::pat::base::is_pattern_v<Pattern>,
        "[Patternia Error] The provided type is not a valid pattern. "
        "A pattern must be invocable with a subject and return a boolean.");
  }

  // Detects unreachable cases in pattern matching (placeholder for future
  // implementation)
  template <typename... Cases>
  struct has_unreachable_case : std::false_type {};

  // Convenience variable template for unreachable case detection
  template <typename... Cases>
  inline constexpr bool has_unreachable_case_v =
      has_unreachable_case<Cases...>::value;

} // namespace ptn::core::common
