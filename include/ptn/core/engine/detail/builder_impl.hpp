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

namespace ptn::core::dsl::detail {
  template <typename... Cases>
  struct on;
} // namespace ptn::core::dsl::detail

namespace ptn::core::engine::detail {

  // Builder class for constructing pattern match expressions.
  // Provides a fluent interface for building match expressions with method
  // chaining.
  // TV is the type of the subject value being matched.
  // Cases are the case-expression types accumulated so far.
  template <typename TV, bool HasMatchFallback, typename... Cases>
  class [[nodiscard(
      "[Patternia.match]: incomplete match expression. "
      "Call .otherwise(...) or .end() to finalize.")]]
  match_builder {

  public:
    using subject_type = TV;
    using cases_type   = std::tuple<Cases...>;

    // Validate subject type (diagnostics centralized in diagnostics.hpp)
    using subject_type_check =
        ptn::core::common::subject_type_validator<subject_type>;

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

    // Unified exhaustiveness predicate.
    static constexpr bool is_exhaustive =
        has_pattern_fallback || has_match_fallback;

    // Chaining API: .when(...)

    // Add a new case (rvalue-qualified).
    // Accepts case expressions created with the '>>' operator.
    // This is the preferred overload for method chaining.
    template <typename CaseExpr>
    constexpr auto when(CaseExpr &&expr) && {

      ptn::core::common::static_assert_when_precondition<
          has_pattern_fallback>();

      using new_case_type = std::decay_t<CaseExpr>;
      ptn::core::common::static_assert_new_case_reachable_after_plain_alt<
          Cases...,
          new_case_type>();

      // Compile-time validation of the new case
      ptn::core::common::
          static_assert_valid_case<new_case_type, subject_type>();

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

    // Pipeline API: match(x) | on{ case1, case2, ... }
    template <typename... NewCases>
    constexpr decltype(auto)
    operator|(core::dsl::detail::on<NewCases...> on_cases) && {
      constexpr bool is_fresh_pipeline = (sizeof...(Cases) == 0);
      static_assert(
          is_fresh_pipeline,
          "[Patternia.on]: pipeline form only supports a fresh match(subject). "
          "Use either match(subject) | on{...} or the chained .when(...) API.");

      (ptn::core::common::static_assert_valid_case<NewCases, subject_type>(),
       ...);

      ptn::core::common::static_assert_no_unreachable_alt_after_plain_alt<
          NewCases...>();

      constexpr bool no_unreachable_cases =
          !ptn::core::common::has_unreachable_case_v<NewCases...>;
      static_assert(
          no_unreachable_cases,
          "[Patternia.on]: case sequence contains unreachable cases. "
          "Tip: ensure wildcard '__' is last and remove shadowed cases.");

      constexpr bool has_pattern_fallback_in_on =
          (traits::is_pattern_fallback_v<traits::case_pattern_t<NewCases>> ||
           ...);
      static_assert(
          has_pattern_fallback_in_on,
          "[Patternia.on]: missing wildcard '__' fallback. "
          "Tip: add '__ >> handler' as the last case.");

      auto dummy_fallback =
          [](auto &&...) -> ptn::core::traits::detail::unreachable_t {
        return {};
      };
      using dummy_handler_t = decltype(dummy_fallback);

      ptn::core::common::
          static_assert_valid_match<subject_type, dummy_handler_t, NewCases...>();

      using result_type = core::traits::
          match_result_t<subject_type, dummy_handler_t, NewCases...>;

      if constexpr (traits::is_void_like_v<result_type>) {
        match_impl::eval<result_type>(
            std::forward<subject_type>(subject_),
            on_cases.cases,
            std::move(dummy_fallback));
      }
      else {
        return match_impl::eval<result_type>(
            std::forward<subject_type>(subject_),
            on_cases.cases,
            std::move(dummy_fallback));
      }
    }

    // Terminal API: .otherwise(...)

    // Terminal step: evaluate all cases; if none matches, call fallback
    // handler. Executes the pattern matching algorithm in sequence order.
    // Handler can be either a value or a callable.
    template <typename Otherwise>
    constexpr decltype(auto) otherwise(Otherwise &&otherwise_handler) && {

      using OtherwiseDecayed = std::decay_t<Otherwise>;

      ptn::core::common::static_assert_otherwise_precondition<
          has_pattern_fallback>();

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
      ptn::core::common::static_assert_end_precondition<
          has_pattern_fallback,
          has_match_fallback>();

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
