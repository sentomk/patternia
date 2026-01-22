#pragma once

// Compile-time diagnostics and static assertions for Patternia.
//
// This header provides validation utilities that detect common pattern matching
// errors at compile time, providing clear error messages to guide developers.
//
#include <type_traits>
#include <variant>

#include "ptn/meta/base/traits.hpp"
#include "ptn/meta/dsa/type_list.hpp"
#include "ptn/meta/query/template_info.hpp"
#include "ptn/core/common/common_traits.hpp"

namespace ptn::core::common {

  // ------------------------------------------------------------
  // Match Expression Validation
  // ------------------------------------------------------------

  // Validates the entire match expression for consistency.
  // Ensures all handlers are invocable and have compatible return types.
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

  // Validates that a handler signature matches a pattern's binding
  // requirements.
  template <typename Case, typename Subject>
  constexpr void static_assert_valid_handler() {
    static_assert(
        traits::is_handler_invocable_v<Case, Subject>,
        "[Patternia Error] Handler signature does not match the pattern's "
        "binding result.");
  }

  // Validates a single case expression structure and handler compatibility.
  template <typename Case, typename Subject>
  constexpr void static_assert_valid_case() {
    static_assert(
        traits::is_case_expr_v<Case>,
        "[Patternia Error] Argument to `.when()` must be a case expression "
        "created with the '>>' operator.");
    static_assert_valid_handler<Case, Subject>();
  }

  // Validates that a type satisfies the pattern requirements.
  template <typename Pattern, typename Subject>
  constexpr void static_assert_valid_pattern() {
    static_assert(
        ptn::pat::base::is_pattern_v<Pattern>,
        "[Patternia Error] The provided type is not a valid pattern. "
        "A pattern must be invocable with a subject and return a boolean.");
  }

  // Detects unreachable cases in pattern matching (placeholder for future
  // implementation).
  template <typename... Cases>
  struct has_unreachable_case : std::false_type {};

  // Convenience variable template for unreachable case detection.
  template <typename... Cases>
  inline constexpr bool has_unreachable_case_v =
      has_unreachable_case<Cases...>::value;

  // ------------------------------------------------------------
  // Builder API Validation
  // ------------------------------------------------------------

  // Checks if the subject type is valid for pattern matching (must be an lvalue
  // reference). This is a variable template because class bodies can only use
  // static_assert with constant boolean conditions, not function calls.
  template <typename Subject>
  inline constexpr bool is_subject_type_valid_v =
      std::is_lvalue_reference_v<Subject>;

  // Validates the preconditions for adding a new case via .when().
  // Ensures that no wildcard pattern ('__') has been added previously,
  // as wildcard matches everything and makes subsequent cases unreachable.
  template <bool HasPatternFallback>
  constexpr void static_assert_when_precondition() {
    static_assert(
        !HasPatternFallback,
        "[Patternia.match]: no cases may follow a wildcard ('__') pattern.");
  }

  // Validates the preconditions for calling .otherwise().
  // Ensures that .otherwise() is not used when a pattern-level fallback ('__')
  // is already present, as the wildcard makes the explicit fallback redundant.
  template <bool HasPatternFallback>
  constexpr void static_assert_otherwise_precondition() {
    static_assert(
        !HasPatternFallback,
        "[Patternia.match]: 'otherwise()' cannot be used when a wildcard "
        "'__' pattern is present. Use '.end()' instead.");
  }

  // Validates the preconditions for calling .end().
  // Ensures:
  // 1. A pattern-level fallback ('__') exists (exhaustive match).
  // 2. No match-level fallback (from .otherwise()) has been added.
  template <bool HasPatternFallback, bool HasMatchFallback>
  constexpr void static_assert_end_precondition() {
    static_assert(
        HasPatternFallback,
        "[Patternia.match.end]: .end() requires a pattern-level fallback "
        "('__'). "
        "If the match is not exhaustive, use .otherwise(...).");

    static_assert(
        !HasMatchFallback,
        "[Patternia.match.end]: .end() cannot be used after otherwise().");
  }

  // Validates that .end() is used in statement-style matches.
  template <typename Result>
  constexpr void static_assert_end_result() {
    static_assert(
        traits::is_void_like_v<Result>,
        "[Patternia.match.end]: .end() is statement-style; handlers must "
        "return void. Use .otherwise(...) for value-returning matches.");
  }

  // ------------------------------------------------------------
  // Variant Diagnostics
  // ------------------------------------------------------------

  namespace detail {
    // Count how many times T appears in a type_list.
    template <typename T, typename TL>
    struct type_count;

    template <typename T, typename... Ts>
    struct type_count<T, meta::type_list<Ts...>>
        : std::integral_constant<
              std::size_t,
              (0 + ... + (std::is_same_v<T, Ts> ? 1 : 0))> {};
  } // namespace detail

  // Ensures Subject is a std::variant specialization.
  template <typename Subject>
  constexpr void static_assert_variant_subject() {
    static_assert(
        meta::is_spec_of_v<std::variant, meta::remove_cvref_t<Subject>>,
        "[Patternia.type::is]: Subject must be a std::variant.");
  }

  // Ensures Alt appears exactly once in the variant's alternatives.
  template <typename Alt, typename Subject>
  constexpr void static_assert_variant_alt_unique() {
    static_assert_variant_subject<Subject>();

    using subject_t = meta::remove_cvref_t<Subject>;
    using args_t    = typename meta::template_info<subject_t>::args;

    constexpr std::size_t count =
        detail::type_count<meta::remove_cvref_t<Alt>, args_t>::value;

    static_assert(
        count == 1,
        "[Patternia.type::is]: Alternative type must appear exactly once in "
        "std::variant.");
  }

} // namespace ptn::core::common
