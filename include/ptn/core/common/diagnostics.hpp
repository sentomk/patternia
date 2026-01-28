#pragma once

// Compile-time diagnostics and static assertions for Patternia.
//
// This header provides validation utilities that detect common pattern matching
// errors at compile time, providing clear error messages to guide developers.
//
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

#include "ptn/meta/base/traits.hpp"
#include "ptn/meta/dsa/type_list.hpp"
#include "ptn/meta/query/template_info.hpp"
#include "ptn/core/common/common_traits.hpp"
#include "ptn/pattern/base/pattern_traits.hpp"

namespace ptn::core::common {

  // ------------------------------------------------------------
  // Case Expression Validation
  // ------------------------------------------------------------

  // Ensures cases(...) only receives case expressions (pattern >> handler).
  template <typename... CaseExprs>
  constexpr void static_assert_cases_are_case_expr() {
    static_assert(
        (ptn::core::traits::is_case_expr_v<CaseExprs> && ...),
        "[Patternia.cases]: arguments must be case expressions created "
        "with '>>'.");
  }

  // ------------------------------------------------------------
  // Match Expression Validation
  // ------------------------------------------------------------

  // Validates the entire match expression for consistency.
  // Ensures all handlers are invocable and have compatible return types.
  template <typename Subject, typename Otherwise, typename... Cases>
  constexpr void static_assert_valid_match() {
    // (A) Each case handler must accept its pattern's bound arguments.
    static_assert(
        (traits::is_handler_invocable_v<Cases, Subject> && ...),
        "[Patternia Error] At least one case's handler cannot be invoked with "
        "the arguments bound by its pattern. "
        "Please check the handler's signature against the pattern's expected "
        "bindings.");

    // (B) The otherwise handler must be callable with Subject or no args.
    static_assert(
        std::is_invocable_v<Otherwise, Subject> ||
            std::is_invocable_v<Otherwise>,
        "[Patternia Error] The `otherwise` handler has an invalid signature. "
        "It should be either callable with the subject value or callable with "
        "no arguments.");

    // (C) All handlers must share a common return type.
    using common_return_type =
        traits::match_result_t<Subject, Otherwise, Cases...>;

    // Force instantiation to catch potential type errors (skip for void)
    if constexpr (!std::is_void_v<common_return_type>) {
      (void) sizeof(common_return_type);
    }
  }

  // Validates that a handler signature matches a pattern's bindings.
  template <typename Case, typename Subject>
  constexpr void static_assert_valid_handler() {
    // (A) Handler must be invocable with the bound values.
    static_assert(
        traits::is_handler_invocable_v<Case, Subject>,
        "[Patternia Error] Handler signature does not match the pattern's "
        "binding result.");
  }

  // Validates a single case expression structure and handler compatibility.
  template <typename Case, typename Subject>
  constexpr void static_assert_valid_case() {
    // (A) A .when(...) must receive a case expression.
    static_assert(
        traits::is_case_expr_v<Case>,
        "[Patternia Error] Argument to `.when()` must be a case expression "
        "created with the '>>' operator.");
    // (B) And its handler must be callable with bound values.
    static_assert_valid_handler<Case, Subject>();
  }

  // Validates that a type satisfies the pattern requirements.
  template <typename Pattern, typename Subject>
  constexpr void static_assert_valid_pattern() {
    // (A) Provided type must satisfy Patternia's pattern contract.
    static_assert(
        ptn::pat::base::is_pattern_v<Pattern>,
        "[Patternia Error] The provided type is not a valid pattern. "
        "A pattern must be invocable with a subject and return a boolean.");
  }

  namespace detail {
    template <typename... Cases>
    struct fallback_is_last : std::true_type {};

    template <typename Case>
    struct fallback_is_last<Case> : std::true_type {};

    template <typename Case, typename Next, typename... Rest>
    struct fallback_is_last<Case, Next, Rest...>
        : std::conditional_t<ptn::core::traits::is_pattern_fallback_v<
                                 ptn::core::traits::case_pattern_t<Case>>,
                             std::false_type,
                             fallback_is_last<Next, Rest...>> {};
  } // namespace detail

  // Detects unreachable cases in pattern matching (placeholder for future
  // implementation).
  template <typename... Cases>
  struct has_unreachable_case
      : std::bool_constant<!detail::fallback_is_last<Cases...>::value> {};

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

  // Emits a diagnostic if subject is not an lvalue reference.
  template <typename Subject>
  struct subject_type_validator {
    static_assert(
        is_subject_type_valid_v<Subject>,
        "[Patternia.match]: subject must be an lvalue reference");
  };

  // Validates the preconditions for adding a new case via .when().
  // Ensures that no wildcard pattern ('__') has been added previously,
  // as wildcard matches everything and makes subsequent cases unreachable.
  template <bool HasPatternFallback>
  constexpr void static_assert_when_precondition() {
    // (A) No cases may follow a wildcard '__' (it matches everything).
    static_assert(
        !HasPatternFallback,
        "[Patternia.match.when]: cannot add cases after wildcard '__'.");
  }

  // Validates the preconditions for calling .otherwise().
  // Ensures that .otherwise() is not used when a pattern-level fallback ('__')
  // is already present, as the wildcard makes the explicit fallback redundant.
  template <bool HasPatternFallback>
  constexpr void static_assert_otherwise_precondition() {
    // (A) .otherwise() is invalid when '__' is already present.
    static_assert(
        !HasPatternFallback,
        "[Patternia.match.otherwise]: wildcard '__' already present. "
        "Use .end() instead.");
  }

  // Validates the preconditions for calling .end().
  // Ensures:
  // 1. A pattern-level fallback ('__') exists (exhaustive match).
  // 2. No match-level fallback (from .otherwise()) has been added.
  template <bool HasPatternFallback, bool HasMatchFallback>
  constexpr void static_assert_end_precondition() {
    // (A) .end() requires a wildcard case for pattern-level fallback.
    static_assert(
        HasPatternFallback,
        "[Patternia.match.end]: missing wildcard '__'. "
        "Use .otherwise(...) for non-exhaustive matches.");

    // (B) .end() cannot follow .otherwise().
    static_assert(
        !HasMatchFallback,
        "[Patternia.match.end]: cannot be used after .otherwise().");
  }

  // Validates that cases(...) only uses non-binding patterns.
  template <typename... Cases>
  constexpr void static_assert_cases_precondition() {
    // (A) cases(...) disallows binding/guards (no binding patterns).
    static_assert(
        (!ptn::pat::traits::is_binding_pattern_v<
             ptn::core::traits::case_pattern_t<Cases>> &&
         ...),
        "[Patternia.cases]: binding/guards are not allowed in cases(...). "
        "Use match(...).when(...) for binding and guard logic.");
    // (B) Wildcard must be last in cases(...).
    static_assert(
        detail::fallback_is_last<Cases...>::value,
        "[Patternia.cases]: wildcard '__' must be the last case.");
  }

  // ------------------------------------------------------------
  // Literal Pattern Validation
  // ------------------------------------------------------------

  // Ensures literal values are storable and comparable.
  template <typename StoreT>
  constexpr void static_assert_literal_store_type() {
    // (A) Literal storage type cannot be void.
    static_assert(
        !std::is_void_v<StoreT>,
        "[Patternia.lit]: Literal value cannot be of type void.");
    // (B) Literal storage must be a value type (not a reference).
    static_assert(
        !std::is_reference_v<StoreT>,
        "[Patternia.lit]: Literal value must be a value type (non-reference).");
    // (C) Literal storage must be movable.
    static_assert(
        std::is_move_constructible_v<StoreT>,
        "[Patternia.lit]: Literal value must be move-constructible.");
    // (D) Literal storage must support equality comparison.
    static_assert(
        std::is_constructible_v<
            bool,
            decltype(std::declval<const StoreT &>() ==
                     std::declval<const StoreT &>())>,
        "[Patternia.lit]: Literal value type must support operator==.");
  }

  // ------------------------------------------------------------
  // Structural Pattern Validation
  // ------------------------------------------------------------

  // Ensures has<...> receives only member pointers or placeholders.
  template <auto... Ms>
  constexpr void static_assert_structural_elements() {
    // (A) has<...> only accepts member pointers or placeholders.
    static_assert(
        (ptn::pat::traits::is_structural_element_v<Ms> && ...),
        "[Patternia.has]: only data member pointers or nullptr/_ign allowed.");
  }

  // ------------------------------------------------------------
  // Guard Predicate Validation
  // ------------------------------------------------------------

  // Ensures arg<I> references are within the bound tuple range.
  template <std::size_t MaxIndex, std::size_t N>
  constexpr void static_assert_tuple_guard_index() {
    // (A) arg<I> must reference an existing bound value.
    static_assert(
        MaxIndex < N,
        "[Patternia.guard]: arg<I> is out of range for the bound values.");
  }

  // Ensures unary guard predicates are only used with single bindings.
  template <std::size_t N>
  constexpr void static_assert_unary_guard_arity() {
    // (A) Unary guards require exactly one bound value.
    static_assert(
        N == 1,
        "[Patternia.guard]: Unary guard predicates (_ / rng / && / ||) "
        "require the pattern to bind exactly ONE value. "
        "For multi-value guards, use a callable predicate (lambda / where).");
  }

  // ------------------------------------------------------------
  // Pattern Bind Contract Validation
  // ------------------------------------------------------------

  // Ensures a pattern exposes bind(subject) for handler invocation.
  template <bool HasBindMember>
  constexpr void static_assert_pattern_has_bind() {
    // (A) Patterns used in handlers must provide bind(subject).
    static_assert(
        HasBindMember,
        "Pattern must have a 'bind(subject)' method that returns a tuple of "
        "bound values.");
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
    // (A) Subject must be a std::variant specialization.
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

    // (A) Alternative type must appear exactly once.
    static_assert(
        count == 1,
        "[Patternia.type::is]: Alternative type must appear exactly once in "
        "std::variant.");
  }

  // Ensures index I is within the variant's alternative range.
  template <std::size_t I, typename Subject>
  constexpr void static_assert_variant_alt_index() {
    static_assert_variant_subject<Subject>();

    using subject_t = meta::remove_cvref_t<Subject>;
    // (A) Alternative index must be in range.
    static_assert(
        I < std::variant_size_v<subject_t>,
        "[Patternia.type::alt]: Alternative index is out of range.");
  }

} // namespace ptn::core::common
