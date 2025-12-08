#pragma once

// Implementation of the Core match_builder.

#include <tuple>
#include <type_traits>
#include <utility>

#include "ptn/config.hpp"
#include "ptn/core/common/common_traits.hpp"
#include "ptn/core/common/diagnostics.hpp"
#include "ptn/core/engine/detail/match_impl.hpp"

namespace ptn::core::engine::detail {

  // Builder class for constructing pattern match expressions.
  // 
  // This class provides a fluent interface for building match expressions
  // with multiple cases using method chaining. It stores the subject to be
  // matched and a tuple of case expressions.
  // 
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

    // ----------------- chaining API: .when(...) -----------------

    // Add a new case (rvalue-qualified).
    //
    // Accepts any CaseExpr-like object; typically `dsl::case_expr<Pattern,
    // Handler>`. This is the preferred overload for method chaining as it
    // moves the builder instance efficiently.
    template <typename CaseExpr>
    constexpr auto when(CaseExpr &&expr) && {
      using new_case_type = std::decay_t<CaseExpr>;

      // Compile-time validation of the new case.
      static_assert(
          ptn::core::common::is_case_expr_v<new_case_type>,
          "Argument to .when() must be a case expression created with the '>>' "
          "operator.");
      static_assert(
          ptn::core::common::
              is_handler_invocable_v<new_case_type, subject_type>,
#if PTN_USE_CONCEPTS
          "Handler cannot be invoked with the arguments bound by the pattern."
#else
          "Handler signature does not match the pattern's binding result."
#endif
      );

      // Concatenate existing cases with the new case
      auto new_cases = std::tuple_cat(
          std::move(cases_),
          std::tuple<new_case_type>(std::forward<CaseExpr>(expr)));

      // Create a new builder type with the additional case
      using builder_t = match_builder<subject_type, Cases..., new_case_type>;

      return builder_t{std::move(subject_), std::move(new_cases)};
    }

    // Add a new case (lvalue-qualified).
    //
    // This overload is used when the builder is accessed as an lvalue.
    // It copies the subject instead of moving it, which is less efficient
    // but necessary for certain use cases.
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
#if PTN_USE_CONCEPTS
          "Handler cannot be invoked with the arguments bound by the pattern."
#else
          "Handler signature does not match the pattern's binding result."
#endif
      );

      // Concatenate existing cases with the new case (copying cases_)
      auto new_cases = std::tuple_cat(
          cases_, std::tuple<new_case_type>(std::forward<CaseExpr>(expr)));

      // Create a new builder type with the additional case
      using builder_t = match_builder<subject_type, Cases..., new_case_type>;

      return builder_t{subject_, std::move(new_cases)};
    }

    // ----------------- terminal API: .otherwise(...) ------------

    // Terminal step: evaluate all cases; if none matches,
    // call the provided fallback handler.
    // 
    // This method executes the pattern matching algorithm. It tries each
    // case in order, and if none matches, it calls the otherwise handler.
    // The handler can be either a value or a callable.
    // 
    // Otherwise: Type of the fallback handler or value
    // otherwise_handler: The fallback to use when no cases match
    // Returns: The result of the matched case handler or the otherwise handler
    template <typename Otherwise>
    constexpr decltype(auto) otherwise(Otherwise &&otherwise_handler) && {

      using OtherwiseDecayed = std::decay_t<Otherwise>;

      // Create a proper handler from the provided argument
      auto final_handler = [&]() {
        if constexpr (ptn::core::common::detail::is_value_like_v<
                          OtherwiseDecayed>) {
          // Case 1: Pass a value - create a handler that returns this value
          return [val = std::forward<Otherwise>(otherwise_handler)](
                     auto &&...) -> OtherwiseDecayed { return val; };
        }
        else {
          // Case 2: Pass a callable - use it directly as the handler
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

      // Additional validation to ensure type consistency
      core::common::static_assert_valid_match<
          subject_type,
          std::decay_t<Otherwise>,
          Cases...>();

      // Execute the match expression
      if constexpr (std::is_void_v<result_type>) {
        // For void return types, just execute without returning a value
        match_impl::eval(
            subject_, cases_, std::forward<Otherwise>(final_handler));
      }
      else {
        // For non-void return types, return the result
        return match_impl::eval(
            subject_, cases_, std::forward<Otherwise>(final_handler));
      }
    }
  };

} // namespace ptn::core::engine::detail
