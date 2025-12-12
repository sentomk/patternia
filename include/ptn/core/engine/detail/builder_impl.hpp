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
  // Provides fluent interface for building match expressions with method chaining.
  // TV: The type of the subject value to be matched
  // Cases: The types of case expressions added so far
  template <typename TV, typename... Cases>
  class match_builder {
  public:
    using subject_type = TV;
    using cases_type   = std::tuple<Cases...>;

  private:
    subject_type subject_; // The value being matched against patterns
    cases_type   cases_;   // Tuple of case expressions

  public:
    // Static factory with an empty case list.
    static constexpr auto create(subject_type subject) {
      return match_builder{std::move(subject), cases_type{}};
    }

    // Static factory with an explicit case tuple (internal use).
    static constexpr auto create(subject_type subject, cases_type cases) {
      return match_builder{std::move(subject), std::move(cases)};
    }

    // Construct from subject and case tuple.
    constexpr match_builder(subject_type subject, cases_type cases)
        : subject_(std::move(subject)), cases_(std::move(cases)) {
    }

    // Chaining API: .when(...)

    // Add a new case (rvalue-qualified).
    // Accepts case expressions created with the '>>' operator.
    // This is the preferred overload for method chaining.
    template <typename CaseExpr>
    constexpr auto when(CaseExpr &&expr) && {
      using new_case_type = std::decay_t<CaseExpr>;

      // Compile-time validation of the new case
      static_assert(
          ptn::core::common::is_case_expr_v<new_case_type>,
          "Argument to .when() must be a case expression created with the '>>' "
          "operator.");
      static_assert(
          ptn::core::common::
              is_handler_invocable_v<new_case_type, subject_type>,
          "Handler signature does not match the pattern's binding result.");

      // Concatenate existing cases with the new case
      auto new_cases = std::tuple_cat(
          std::move(cases_),
          std::tuple<new_case_type>(std::forward<CaseExpr>(expr)));

      // Create a new builder type with the additional case
      using builder_t = match_builder<subject_type, Cases..., new_case_type>;

      return builder_t{std::move(subject_), std::move(new_cases)};
    }

    // Add a new case (lvalue-qualified).
    // Used when the builder is accessed as an lvalue.
    // Less efficient but necessary for certain use cases.
    template <typename CaseExpr>
    constexpr auto when(CaseExpr &&expr) const & {
      using new_case_type = std::decay_t<CaseExpr>;

      static_assert(
          ptn::core::common::is_case_expr_v<new_case_type>,
          "Argument to .when() must be a case expression created with the '>>' "
          "operator.");
      static_assert(
          ptn::core::common::
              is_handler_invocable_v<new_case_type, subject_type>,
          "Handler signature does not match the pattern's binding result.");

      // Concatenate existing cases with the new case (copying cases_)
      auto new_cases = std::tuple_cat(
          cases_, std::tuple<new_case_type>(std::forward<CaseExpr>(expr)));

      // Create a new builder type with the additional case
      using builder_t = match_builder<subject_type, Cases..., new_case_type>;

      return builder_t{subject_, std::move(new_cases)};
    }

    // Terminal API: .otherwise(...)

    // Terminal step: evaluate all cases; if none matches, call fallback handler.
    // Executes the pattern matching algorithm in sequence order.
    // Handler can be either a value or a callable.
    template <typename Otherwise>
    constexpr decltype(auto) otherwise(Otherwise &&otherwise_handler) && {

      using OtherwiseDecayed = std::decay_t<Otherwise>;

      // Create a proper handler from the provided argument
      auto final_handler = [&]() {
        if constexpr (ptn::core::common::detail::is_value_like_v<
                          OtherwiseDecayed>) {
          // Case 1: Value - create a handler that returns this value
          return [val = std::forward<Otherwise>(otherwise_handler)](
                     auto &&...) -> OtherwiseDecayed { return val; };
        }
        else {
          // Case 2: Callable - use it directly as handler
          return std::forward<Otherwise>(otherwise_handler);
        }
      }();

      // Validate that the match expression is well-formed
      ptn::core::common::static_assert_valid_match<
          subject_type,
          decltype(final_handler),
          Cases...>();

      // Determine the result type of the entire match expression
      using result_type = core::common::
          match_result_t<subject_type, std::decay_t<Otherwise>, Cases...>;

      // Execute the match expression
      if constexpr (std::is_void_v<result_type>) {
        // For void return types, execute without returning a value
        match_impl::eval(
            subject_, cases_, std::forward<Otherwise>(final_handler));
      }
      else {
        // For non-void return types, return the result
        return match_impl::eval(
            subject_, cases_, std::forward<Otherwise>(final_handler));
      }
    }

    // Terminal API: .end()
    // For void-only match expressions with no explicit otherwise handler.
    constexpr void end() && {
      // Create a dummy fallback handler (callable, never used if exhaustive)
      auto dummy_fallback = []() {};
      using dummy_handler_t = decltype(dummy_fallback);

      // Validate match result type
      ptn::core::common::
          static_assert_valid_match<subject_type, dummy_handler_t, Cases...>();

      // Compute the match result type (should be void)
      using result_type = core::common::
          match_result_t<subject_type, dummy_handler_t, Cases...>();

      static_assert(
          common::is_void_like_v<result_type>,
          "[Patternia.match.end]: .end() requires all case handlers to return "
          "void."
          "If you want a value-returning match, use "
          ".otherwise(value_or_lambda).");

      // Execute match evaluation
      match_impl::eval(subject_, cases_, std::move(dummy_fallback));
    }
  };
} // namespace ptn::core::engine::detail
