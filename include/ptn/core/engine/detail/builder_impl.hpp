#pragma once

// Implementation of the Core match_builder.
//
// This header contains the internal implementation of Patternia's fluent
// builder pattern for constructing pattern matching expressions.

#include <algorithm>
#include <tuple>
#include <type_traits>
#include <utility>

#include "ptn/core/common/common_traits.hpp"
#include "ptn/core/common/diagnostics.hpp"
#include "ptn/core/engine/detail/match_impl.hpp"

namespace ptn::core::engine::detail {

  // Builder class for constructing pattern match expressions.
  // Provides fluent interface for building match expressions with method
  // chaining. TV: The type of the subject value to be matched Cases: The types
  // of case expressions added so far
  template <typename TV, bool HasMatchFallback, typename... Cases>
  class [[nodiscard(
      "[Patternia.match]: incomplete match expression. "
      "Call .otherwise(...) or .end() to finalize.")]]
  match_builder {

  public:
    using subject_type = TV;
    using cases_type   = std::tuple<Cases...>;

    static_assert(
        std::is_lvalue_reference_v<subject_type>,
        "Patternia.match: subject must be an lvalue reference");

  private:
    subject_type subject_; // The value being matched against patterns
    cases_type   cases_;   // Tuple of case expressions

  public:
    // Static factory with an empty case list.
    static constexpr auto create(subject_type subject) {
      return match_builder{std::forward<subject_type>(subject), cases_type{}};
    }

    // Static factory with an explicit case tuple (internal use).
    static constexpr auto create(subject_type subject, cases_type cases) {
      return match_builder{
          std::forward<subject_type>(subject), std::move(cases)};
    }

    // Construct from subject and case tuple.
    constexpr match_builder(subject_type subject, cases_type cases)
        : subject_(std::forward<subject_type>(subject)),
          cases_(std::move(cases)) {
    }

    // Triggered by .when(__)
    static constexpr bool has_pattern_fallback =
        (traits::is_pattern_fallback_v<traits::case_pattern_t<Cases>> || ...);

    // Triggered by .otherwise()
    static constexpr bool has_match_fallback = HasMatchFallback;

    // unified exhaustiveness predicate
    static constexpr bool is_exhaustive =
        has_pattern_fallback || has_match_fallback;

    // Chaining API: .when(...)

    // Add a new case (rvalue-qualified).
    // Accepts case expressions created with the '>>' operator.
    // This is the preferred overload for method chaining.
    template <typename CaseExpr>
    constexpr auto when(CaseExpr &&expr) && {

      static_assert(
          !has_pattern_fallback,
          "[Patternia.match]: no cases may follow a wildcard ('__') pattern.");

      using new_case_type = std::decay_t<CaseExpr>;

      // Compile-time validation of the new case
      static_assert(
          ptn::core::traits::is_case_expr_v<new_case_type>,
          "Argument to .when() must be a case expression created with the '>>' "
          "operator.");
      static_assert(
          ptn::core::traits::
              is_handler_invocable_v<new_case_type, subject_type>,
          "Handler signature does not match the pattern's binding result.");

      // Concatenate existing cases with the new case
      auto new_cases = std::tuple_cat(
          std::move(cases_),
          std::tuple<new_case_type>(std::forward<CaseExpr>(expr)));

      // Create a new builder type with the additional case
      using builder_t = match_builder<
          subject_type,
          HasMatchFallback,
          Cases...,
          new_case_type>;

      return builder_t{
          std::forward<subject_type>(subject_), std::move(new_cases)};
    }

    // Add a new case (lvalue-qualified).
    // Used when the builder is accessed as an lvalue.
    // Less efficient but necessary for certain use cases.
    template <typename CaseExpr>
    constexpr auto when(CaseExpr &&expr) const & {
      using new_case_type = std::decay_t<CaseExpr>;

      static_assert(
          ptn::core::traits::is_case_expr_v<new_case_type>,
          "Argument to .when() must be a case expression created with the '>>' "
          "operator.");
      static_assert(
          ptn::core::traits::
              is_handler_invocable_v<new_case_type, subject_type>,
          "Handler signature does not match the pattern's binding result.");

      // Concatenate existing cases with the new case (copying cases_)
      auto new_cases = std::tuple_cat(
          cases_, std::tuple<new_case_type>(std::forward<CaseExpr>(expr)));

      // Create a new builder type with the additional case
      using builder_t = match_builder<
          subject_type,
          HasMatchFallback,
          Cases...,
          new_case_type>;

      return builder_t{
          std::forward<subject_type>(subject_), std::move(new_cases)};
    }

    // Terminal API: .otherwise(...)

    // Terminal step: evaluate all cases; if none matches, call fallback
    // handler. Executes the pattern matching algorithm in sequence order.
    // Handler can be either a value or a callable.
    template <typename Otherwise>
    constexpr decltype(auto) otherwise(Otherwise &&otherwise_handler) && {

      using OtherwiseDecayed = std::decay_t<Otherwise>;

      static_assert(
          !has_pattern_fallback,
          "[Patternia.match]: 'otherwise()' cannot be used when a wildcard "
          "'__' pattern is present. Use '.end()' instead.");

      // Create a proper handler from the provided argument
      auto final_handler = [&]() {
        if constexpr (ptn::core::traits::detail::is_value_like_v<
                          OtherwiseDecayed>) {
          // Case 1: Value - create a handler that returns this value
          return [val = std::forward<Otherwise>(otherwise_handler)](
                     auto &&...) -> OtherwiseDecayed { return val; };
        }
        else {
          // Case 2: Callable - normalize to an object (wrap function
          // pointers/references)
          if constexpr (
              std::is_function_v<std::remove_pointer_t<OtherwiseDecayed>> ||
              std::is_pointer_v<OtherwiseDecayed>) {
            auto fp = std::forward<Otherwise>(otherwise_handler);
            return [fp](auto &&...xs) -> decltype(auto) {
              return fp(std::forward<decltype(xs)>(xs)...);
            };
          }
          else {
            return std::forward<Otherwise>(otherwise_handler);
          }
        }
      }();

      // Validate that the match expression is well-formed
      ptn::core::common::static_assert_valid_match<
          subject_type,
          decltype(final_handler),
          Cases...>();

      // Determine the result type of the entire match expression
      using result_type = core::traits::
          match_result_t<subject_type, decltype(final_handler), Cases...>;

      // Execute the match expression
      if constexpr (std::is_void_v<result_type>) {
        // For void return types, execute without returning a value
        match_impl::eval<void>(
            std::forward<subject_type>(subject_),
            cases_,
            std::forward<decltype(final_handler)>(final_handler));
      }
      else {
        // For non-void return types, return the result
        return match_impl::eval<result_type>(
            std::forward<subject_type>(subject_),
            cases_,
            std::forward<decltype(final_handler)>(final_handler));
      }
    }

    // Terminal API: .end()
    // Evaluate an exhaustive match finalized by a pattern-level fallback (__).
    constexpr auto end() && {
      static_assert(
          has_pattern_fallback,
          "[Patternia.match.end]: .end() requires a pattern-level fallback "
          "('__'). "
          "If the match is not exhaustive, use .otherwise(...).");

      static_assert(
          !has_match_fallback,
          "[Patternia.match.end]: .end() cannot be used after otherwise().");

      // Dummy fallback handler (logically unreachable because '__' exists)
      auto dummy_fallback =
          [](auto &&...) -> ptn::core::traits::detail::unreachable_t {
        return {};
      };
      using dummy_handler_t = decltype(dummy_fallback);

      // Validate match well-formedness
      ptn::core::common::
          static_assert_valid_match<subject_type, dummy_handler_t, Cases...>();

      // Compute match result type (now won't be polluted by 'void')
      using result_type =
          core::traits::match_result_t<subject_type, dummy_handler_t, Cases...>;

      if constexpr (traits::is_void_like_v<result_type>) {
        match_impl::eval<result_type>(
            std::forward<subject_type>(subject_),
            cases_,
            std::move(dummy_fallback));
      }
      else {
        return match_impl::eval<result_type>(
            std::forward<subject_type>(subject_),
            cases_,
            std::move(dummy_fallback));
      }
    }
  };
} // namespace ptn::core::engine::detail

// Forward Declaration
namespace ptn::core::dsl::detail {
  template <typename... Cases>
  struct cases_pack;
}

namespace ptn::core::engine {

  template <typename TV, typename... Cases>
  constexpr auto
  match(TV &subject, core::dsl::detail::cases_pack<Cases...> pack) {

    using subject_ref_t = std::remove_reference_t<TV> &;
    using builder_t     = detail::match_builder<subject_ref_t, false, Cases...>;
    return builder_t::create(subject, std::move(pack.cases));
  }

} // namespace ptn::core::engine
