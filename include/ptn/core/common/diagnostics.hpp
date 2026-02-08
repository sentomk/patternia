#pragma once

// Compile-time diagnostics and static assertions for Patternia.
//
// This header provides validation utilities that detect common pattern matching
// errors at compile time, providing clear error messages to guide developers.
//
#include <type_traits>
#include <utility>
#include <variant>

#include "ptn/meta/base/traits.hpp"
#include "ptn/meta/dsa/type_list.hpp"
#include "ptn/meta/query/template_info.hpp"
#include "ptn/core/common/common_traits.hpp"
#include "ptn/pattern/base/pattern_traits.hpp"

namespace ptn::pat::type::detail {
  struct no_subpattern;

  template <std::size_t I, typename SubPattern>
  struct type_alt_pattern;
} // namespace ptn::pat::type::detail

namespace ptn::pat::mod {
  template <typename Inner, typename Pred>
  struct guarded_pattern;
} // namespace ptn::pat::mod

namespace ptn::core::common {

  // ------------------------------------------------------------
  // Case Expression Validation
  // ------------------------------------------------------------

  // Ensures cases(...) only receives case expressions (pattern >> handler).
  template <typename... CaseExprs>
  constexpr void static_assert_cases_are_case_expr() {
    constexpr bool all_are_case_exprs =
        (ptn::core::traits::is_case_expr_v<CaseExprs> && ...);
    static_assert(
        all_are_case_exprs,
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
    constexpr bool handlers_ok =
        (traits::is_handler_invocable_v<Cases, Subject> && ...);
    static_assert(
        handlers_ok,
        "[Patternia.match] At least one case's handler cannot be invoked with "
        "the arguments bound by its pattern. "
        "Please check the handler's signature against the pattern's expected "
        "bindings. Tip: ensure handler parameter types match the pattern's "
        "bindings.");

    // (B) The otherwise handler must be callable with Subject or no args.
    constexpr bool otherwise_ok = std::is_invocable_v<Otherwise, Subject> ||
                                  std::is_invocable_v<Otherwise>;
    static_assert(
        otherwise_ok,
        "[Patternia.match] The `otherwise` handler has an invalid signature. "
        "It should be either callable with the subject value or callable with "
        "no arguments. Tip: make it callable with Subject or with no args.");

    // (C) All handlers must share a common return type.
    using common_return_type = traits::
        match_result_t<Subject, Otherwise, Cases...>;

    // Force instantiation to catch potential type errors (skip for void)
    if constexpr (!std::is_void_v<common_return_type>) {
      (void) sizeof(common_return_type);
    }
  }

  // Validates that a handler signature matches a pattern's bindings.
  template <typename Case, typename Subject>
  constexpr void static_assert_valid_handler() {
    // (A) Handler must be invocable with the bound values.
    constexpr bool handler_invocable =
        traits::is_handler_invocable_v<Case, Subject>;
    static_assert(
        handler_invocable,
        "[Patternia.match] Handler signature does not match the pattern's "
        "binding result.");
  }

  // Validates a single case expression structure and handler compatibility.
  template <typename Case, typename Subject>
  constexpr void static_assert_valid_case() {
    // (A) A .when(...) must receive a case expression.
    constexpr bool is_case_expr = traits::is_case_expr_v<Case>;
    static_assert(
        is_case_expr,
        "[Patternia.match] Argument to `.when()` must be a case expression "
        "created with the '>>' operator.");
    // (B) And its handler must be callable with bound values.
    static_assert_valid_handler<Case, Subject>();
  }

  // Validates that a type satisfies the pattern requirements.
  template <typename Pattern, typename Subject>
  constexpr void static_assert_valid_pattern() {
    // (A) Provided type must satisfy Patternia's pattern contract.
    constexpr bool is_pattern_type = ptn::pat::base::is_pattern_v<Pattern>;
    static_assert(
        is_pattern_type,
        "[Patternia.match] The provided type is not a valid pattern. "
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

  // Detects unreachable cases in pattern matching.
  namespace detail {
    inline constexpr std::size_t alt_npos = static_cast<std::size_t>(-1);

    template <typename Pattern>
    struct alt_pattern_index : std::integral_constant<std::size_t, alt_npos> {};

    template <std::size_t I, typename SubPattern>
    struct alt_pattern_index<ptn::pat::type::detail::type_alt_pattern<
        I,
        SubPattern>> : std::integral_constant<std::size_t, I> {};

    template <typename Inner, typename Pred>
    struct alt_pattern_index<ptn::pat::mod::guarded_pattern<Inner, Pred>>
        : alt_pattern_index<Inner> {};

    template <typename Pattern>
    struct plain_alt_pattern_index
        : std::integral_constant<std::size_t, alt_npos> {};

    template <std::size_t I>
    struct plain_alt_pattern_index<ptn::pat::type::detail::type_alt_pattern<
        I,
        ptn::pat::type::detail::no_subpattern>>
        : std::integral_constant<std::size_t, I> {};

    template <typename Case>
    inline constexpr std::size_t alt_case_index_v =
        alt_pattern_index<ptn::core::traits::case_pattern_t<Case>>::value;

    template <typename Case>
    inline constexpr std::size_t plain_alt_case_index_v =
        plain_alt_pattern_index<ptn::core::traits::case_pattern_t<Case>>::value;

    template <std::size_t... Is>
    struct alt_index_set {};

    template <std::size_t I, typename Set>
    struct alt_index_set_contains;

    template <std::size_t I, std::size_t... Is>
    struct alt_index_set_contains<I, alt_index_set<Is...>>
        : std::bool_constant<((I == Is) || ...)> {};

    template <std::size_t I, typename Set>
    struct alt_index_set_insert;

    template <std::size_t I, std::size_t... Is>
    struct alt_index_set_insert<I, alt_index_set<Is...>> {
      using type = std::conditional_t<
          alt_index_set_contains<I, alt_index_set<Is...>>::value,
          alt_index_set<Is...>,
          alt_index_set<Is..., I>>;
    };

    template <typename CoveredSet, typename... Cases>
    struct has_unreachable_alt_after_plain_alt_from : std::false_type {};

    template <std::size_t... Covered, typename First, typename... Rest>
    struct has_unreachable_alt_after_plain_alt_from<
        alt_index_set<Covered...>,
        First,
        Rest...>
        : std::bool_constant<
              (((alt_case_index_v<First> != alt_npos) &&
                alt_index_set_contains<
                    alt_case_index_v<First>,
                    alt_index_set<Covered...>>::value)) ||
              has_unreachable_alt_after_plain_alt_from<
                  std::conditional_t<
                      (plain_alt_case_index_v<First> == alt_npos),
                      alt_index_set<Covered...>,
                      typename alt_index_set_insert<
                          plain_alt_case_index_v<First>,
                          alt_index_set<Covered...>>::type>,
                  Rest...>::value> {};

    template <typename... Cases>
    struct has_unreachable_alt_after_plain_alt
        : has_unreachable_alt_after_plain_alt_from<alt_index_set<>, Cases...> {};

    template <typename CoveredSet, typename... Cases>
    struct is_tail_case_unreachable_after_plain_alt : std::false_type {};

    template <typename CoveredSet, typename Last>
    struct is_tail_case_unreachable_after_plain_alt<CoveredSet, Last>
        : std::bool_constant<
              ((alt_case_index_v<Last> != alt_npos) &&
               alt_index_set_contains<
                   alt_case_index_v<Last>,
                   CoveredSet>::value)> {};

    template <
        std::size_t... Covered,
        typename First,
        typename Next,
        typename... Rest>
    struct is_tail_case_unreachable_after_plain_alt<
        alt_index_set<Covered...>,
        First,
        Next,
        Rest...>
        : is_tail_case_unreachable_after_plain_alt<
              std::conditional_t<
                  (plain_alt_case_index_v<First> == alt_npos),
                  alt_index_set<Covered...>,
                  typename alt_index_set_insert<
                      plain_alt_case_index_v<First>,
                      alt_index_set<Covered...>>::type>,
              Next,
              Rest...> {};

    template <typename... Cases>
    struct is_new_case_unreachable_after_plain_alt
        : is_tail_case_unreachable_after_plain_alt<alt_index_set<>, Cases...> {
    };
  } // namespace detail

  template <typename... Cases>
  struct has_unreachable_case
      : std::bool_constant<
            !detail::fallback_is_last<Cases...>::value ||
            detail::has_unreachable_alt_after_plain_alt<Cases...>::value> {};

  // Convenience variable template for unreachable case detection.
  template <typename... Cases>
  inline constexpr bool
      has_unreachable_case_v = has_unreachable_case<Cases...>::value;

  // Emits diagnostic for deterministic unreachable alt<I>(...) cases caused by
  // an earlier plain alt<I>() that already covers the same alternative.
  template <typename... Cases>
  constexpr void static_assert_no_unreachable_alt_after_plain_alt() {
    constexpr bool no_unreachable_alt_after_plain_alt =
        !detail::has_unreachable_alt_after_plain_alt<Cases...>::value;
    static_assert(
        no_unreachable_alt_after_plain_alt,
        "[Patternia.type::alt]: case is unreachable because an earlier "
        "plain type::alt<I>() already covers this alternative. Tip: remove "
        "the later alt<I>(...) case, or reorder cases.");
  }

  // Emits diagnostic only for the newly appended case in builder .when(...),
  // to avoid cascading errors on subsequent chained cases.
  template <typename... Cases>
  constexpr void static_assert_new_case_reachable_after_plain_alt() {
    constexpr bool new_case_reachable =
        !detail::is_new_case_unreachable_after_plain_alt<Cases...>::value;
    static_assert(
        new_case_reachable,
        "[Patternia.type::alt]: case is unreachable because an earlier "
        "plain type::alt<I>() already covers this alternative. Tip: remove "
        "the later alt<I>(...) case, or reorder cases.");
  }

  // ------------------------------------------------------------
  // Builder API Validation
  // ------------------------------------------------------------

  // Checks if the subject type is valid for pattern matching (must be an lvalue
  // reference). This is a variable template because class bodies can only use
  // static_assert with constant boolean conditions, not function calls.
  template <typename Subject>
  inline constexpr bool
      is_subject_type_valid_v = std::is_lvalue_reference_v<Subject>;

  // Emits a diagnostic if subject is not an lvalue reference.
  template <typename Subject>
  struct subject_type_validator {
    static constexpr bool is_lvalue_ref = is_subject_type_valid_v<Subject>;
    static_assert(is_lvalue_ref,
                  "[Patternia.match]: subject must be an lvalue reference. "
                  "Tip: pass an lvalue (match(v)), not std::move(v).");
  };

  // Validates the preconditions for adding a new case via .when().
  // Ensures that no wildcard pattern ('__') has been added previously,
  // as wildcard matches everything and makes subsequent cases unreachable.
  template <bool HasPatternFallback>
  constexpr void static_assert_when_precondition() {
    // (A) No cases may follow a wildcard '__' (it matches everything).
    constexpr bool can_add_after_wildcard = !HasPatternFallback;
    static_assert(
        can_add_after_wildcard,
        "[Patternia.match.when]: cannot add cases after wildcard '__'. "
        "Tip: move '__' to the last case.");
  }

  // Validates the preconditions for calling .otherwise().
  // Ensures that .otherwise() is not used when a pattern-level fallback ('__')
  // is already present, as the wildcard makes the explicit fallback redundant.
  template <bool HasPatternFallback>
  constexpr void static_assert_otherwise_precondition() {
    // (A) .otherwise() is invalid when '__' is already present.
    constexpr bool no_wildcard_present = !HasPatternFallback;
    static_assert(no_wildcard_present,
                  "[Patternia.match.otherwise]: wildcard '__' already present. "
                  "Use .end() instead. Tip: remove .otherwise() when '__' is "
                  "present.");
  }

  // Validates the preconditions for calling .end().
  // Ensures:
  // 1. A pattern-level fallback ('__') exists (exhaustive match).
  // 2. No match-level fallback (from .otherwise()) has been added.
  template <bool HasPatternFallback, bool HasMatchFallback>
  constexpr void static_assert_end_precondition() {
    // (A) .end() requires a wildcard case for pattern-level fallback.
    constexpr bool has_fallback = HasPatternFallback;
    static_assert(has_fallback,
                  "[Patternia.match.end]: missing wildcard '__'. "
                  "Use .otherwise(...) for non-exhaustive matches. Tip: add "
                  "a '__' case before calling .end().");

    // (B) .end() cannot follow .otherwise().
    constexpr bool no_match_fallback = !HasMatchFallback;
    static_assert(no_match_fallback,
                  "[Patternia.match.end]: cannot be used after .otherwise(). "
                  "Tip: remove .otherwise() when using .end().");
  }

  // Validates that cases(...) only uses non-binding patterns.
  template <typename... Cases>
  constexpr void static_assert_cases_precondition() {
    // (A) cases(...) disallows binding/guards (no binding patterns).
    constexpr bool no_bindings =
        (!ptn::pat::traits::is_binding_pattern_v<
             ptn::core::traits::case_pattern_t<Cases>>
         && ...);
    static_assert(
        no_bindings,
        "[Patternia.cases]: binding/guards are not allowed in cases(...). "
        "Use match(...).when(...) for binding and guard logic.");
    // (B) Wildcard must be last in cases(...).
    constexpr bool fallback_last = detail::fallback_is_last<Cases...>::value;
    static_assert(fallback_last,
                  "[Patternia.cases]: wildcard '__' must be the last case. "
                  "Tip: move '__' to the end.");
  }

  // ------------------------------------------------------------
  // Literal Pattern Validation
  // ------------------------------------------------------------

  // Ensures literal values are storable and comparable.
  template <typename StoreT>
  constexpr void static_assert_literal_store_type() {
    // (A) Literal storage type cannot be void.
    constexpr bool not_void = !std::is_void_v<StoreT>;
    static_assert(not_void,
                  "[Patternia.lit]: Literal value cannot be of type void.");
    // (B) Literal storage must be a value type (not a reference).
    constexpr bool not_ref = !std::is_reference_v<StoreT>;
    static_assert(
        not_ref,
        "[Patternia.lit]: Literal value must be a value type (non-reference).");
    // (C) Literal storage must be movable.
    constexpr bool movable = std::is_move_constructible_v<StoreT>;
    static_assert(movable,
                  "[Patternia.lit]: Literal value must be move-constructible.");
    // (D) Literal storage must support equality comparison.
    constexpr bool comparable =
        std::is_constructible_v<bool,
                                decltype(std::declval<const StoreT &>()
                                         == std::declval<const StoreT &>())>;
    static_assert(comparable,
                  "[Patternia.lit]: Literal value type must support operator==.");
  }

  // ------------------------------------------------------------
  // Structural Pattern Validation
  // ------------------------------------------------------------

  // Ensures has<...> receives only member pointers or placeholders.
  template <auto... Ms>
  constexpr void static_assert_structural_elements() {
    // (A) has<...> only accepts member pointers or placeholders.
    constexpr bool all_elements_valid =
        (ptn::pat::traits::is_structural_element_v<Ms> && ...);
    static_assert(
        all_elements_valid,
        "[Patternia.has]: only data member pointers or nullptr/_ign allowed.");
  }

  // Checks whether a subject exposes the requested member pointer.
  // nullptr/_ign placeholders are treated as "ignored slots".
  namespace detail {
    template <typename Subject, auto M, typename = void>
    struct has_member_access : std::false_type {};

    template <typename Subject, auto M>
    struct has_member_access<
        Subject,
        M,
        std::void_t<decltype(std::declval<Subject &>().*M)>> : std::true_type {
    };
  } // namespace detail

  // Validates that a subject type provides all requested members in has<...>.
  template <typename Subject, auto... Ms>
  constexpr void static_assert_structural_accessible() {
    constexpr bool all_members_accessible =
        ((ptn::pat::traits::is_nullptr_placeholder_v<Ms> ||
          detail::has_member_access<Subject, Ms>::value) &&
         ...);
    static_assert(
        all_members_accessible,
        "[Patternia.has]: Subject does not expose one or more requested "
        "members.");
  }

  // ------------------------------------------------------------
  // Guard Predicate Validation
  // ------------------------------------------------------------

  // Ensures arg<I> references are within the bound tuple range.
  template <std::size_t MaxIndex, std::size_t N>
  constexpr void static_assert_tuple_guard_index() {
    // (A) arg<I> must reference an existing bound value.
    constexpr bool guard_index_in_range = MaxIndex < N;
    static_assert(
        guard_index_in_range,
        "[Patternia.guard]: arg<I> is out of range for the bound values.");
  }

  // Ensures unary guard predicates are only used with single bindings.
  template <std::size_t N>
  constexpr void static_assert_unary_guard_arity() {
    // (A) Unary guards require exactly one bound value.
    constexpr bool unary_guard_requires_one_binding = N == 1;
    static_assert(
        unary_guard_requires_one_binding,
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
    constexpr bool pattern_has_bind_method = HasBindMember;
    static_assert(
        pattern_has_bind_method,
        "[Patternia.match] Pattern must have a 'bind(subject)' method that "
        "returns a tuple of bound values.");
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
        : std::integral_constant<std::size_t,
                                 (0 + ... + (std::is_same_v<T, Ts> ? 1 : 0))> {
    };
  } // namespace detail

  // Ensures Subject is a std::variant specialization.
  template <typename Subject>
  constexpr void static_assert_variant_subject() {
    // (A) Subject must be a std::variant specialization.
    constexpr bool is_variant_subject =
        meta::is_spec_of_v<std::variant, meta::remove_cvref_t<Subject>>;
    static_assert(
        is_variant_subject,
        "[Patternia.type::is]: Subject must be a std::variant.");
  }

  // Ensures Alt appears exactly once in the variant's alternatives.
  template <typename Alt, typename Subject>
  constexpr void static_assert_variant_alt_unique() {
    static_assert_variant_subject<Subject>();

    using subject_t = meta::remove_cvref_t<Subject>;
    using args_t    = typename meta::template_info<subject_t>::args;

    constexpr std::size_t
        count = detail::type_count<meta::remove_cvref_t<Alt>, args_t>::value;

    // (A) Alternative type must appear exactly once.
    constexpr bool alt_appears_once = (count == 1);
    static_assert(
        alt_appears_once,
        "[Patternia.type::is]: Alternative type must appear exactly once in "
        "std::variant. Tip: use type::alt<I>() for duplicate types, or wrap "
        "types in distinct structs.");
  }

  // Ensures index I is within the variant's alternative range.
  template <std::size_t I, typename Subject>
  constexpr void static_assert_variant_alt_index() {
    static_assert_variant_subject<Subject>();

    using subject_t = meta::remove_cvref_t<Subject>;
    // (A) Alternative index must be in range.
    constexpr bool alt_index_in_range = I < std::variant_size_v<subject_t>;
    static_assert(alt_index_in_range,
                  "[Patternia.type::alt]: Alternative index is out of range. "
                  "Tip: valid indices are [0, variant_size-1].");
  }

} // namespace ptn::core::common
