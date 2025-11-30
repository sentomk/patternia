#pragma once

/**
 * @file diagnostics.hpp
 * @brief Compile-time diagnostics and static assertions for Patternia.
 */

#include <type_traits>
#include "ptn/core/common/common_traits.hpp"

namespace ptn::core::common {

  /**
   * @brief Validates the entire match expression for consistency.
   */
  template <typename Subject, typename Otherwise, typename... Cases>
  constexpr void static_assert_valid_match() {
    // Check: All case handlers must be invocable with their pattern's bindings.
    static_assert(
        (is_handler_invocable_v<Cases, Subject> && ...),
        "[Patternia Error] At least one case's handler cannot be invoked with "
        "the arguments bound by its pattern. "
        "Please check the handler's signature against the pattern's expected "
        "bindings.");

    // Check: The `otherwise` handler must be invocable.
    static_assert(
        std::is_invocable_v<Otherwise, Subject> ||
            std::is_invocable_v<Otherwise>,
        "[Patternia Error] The `otherwise` handler has an invalid signature. "
        "It should be either callable with the subject value or callable with "
        "no arguments.");

    // Check: All handlers must have a common return type.
    using common_return_type = match_result_t<Subject, Otherwise, Cases...>;
    (void) sizeof(common_return_type);
  }

  /**
   * @brief Validates that a handler matches a pattern's bindings.
   */
  template <typename Case, typename Subject>
  constexpr void static_assert_valid_handler() {
    static_assert(
        is_handler_invocable_v<Case, Subject>,
#if PTN_USE_CONCEPTS
        "[Patternia Error] Handler cannot be invoked with the arguments bound "
        "by the pattern."
#else
        "[Patternia Error] Handler signature does not match the pattern's "
        "binding result."
#endif
    );
  }

  /**
   * @brief Validates a single case expression.
   */
  template <typename Case, typename Subject>
  constexpr void static_assert_valid_case() {
    static_assert(
        is_case_expr_v<Case>,
        "[Patternia Error] Argument to `.when()` must be a case expression "
        "created with the '>>' operator.");
    static_assert_valid_handler<Case, Subject>();
  }

  /**
   * @brief Validates that a type is a valid pattern.
   */
  template <typename Pattern, typename Subject>
  constexpr void static_assert_valid_pattern() {
#if PTN_USE_CONCEPTS
    static_assert(
        ptn::pattern::detail::pattern<Pattern>,
        "[Patternia Error] The provided type is not a valid pattern. "
        "A pattern must be invocable with a subject and return a boolean.");
#else
    static_assert(
        ptn::pat::base::is_pattern_v<Pattern>,
        "[Patternia Error] The provided type is not a valid pattern. "
        "A pattern must be invocable with a subject and return a boolean.");
#endif
  }

  template <typename... Cases>
  struct has_unreachable_case : std::false_type {};

  template <typename... Cases>
  inline constexpr bool has_unreachable_case_v =
      has_unreachable_case<Cases...>::value;

} // namespace ptn::core::common
