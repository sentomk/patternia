#pragma once

// Entry function for Core matching engine.
//
// This header provides the main entry points for Patternia's pattern
// matching functionality, supporting both type-deduced and
// explicit-type matching.

#include <tuple>
#include <type_traits>
#include <utility>

#include "ptn/core/engine/detail/pipeline_match_context.hpp"

namespace ptn {

  namespace detail {

    // Evaluates a match expression with variadic cases.
    //
    // This is the internal implementation for the unified
    // match(subject, cases...) entry point. It validates cases,
    // computes the result type, and delegates to the core evaluation
    // engine.
    template <typename Subject, typename... Cases>
    constexpr decltype(auto) eval_match_cases(Subject &subject,
                                              Cases &&...cases) {
      using subject_type = Subject &;

      // Validate each case against the subject type.
      (core::common::static_assert_valid_case<std::decay_t<Cases>,
                                              subject_type>(),
       ...);

      // Check for unreachable cases after wildcard.
      core::common::static_assert_no_unreachable_alt_after_plain_alt<
          std::decay_t<Cases>...>();

      constexpr bool
          no_unreachable_cases = !core::common::
                                     has_unreachable_case_v<
                                         std::decay_t<Cases>...>;
      static_assert(
          no_unreachable_cases,
          "[Patternia.match]: case sequence contains unreachable "
          "cases. "
          "Tip: ensure wildcard is last and remove shadowed cases.");

      // Require a pattern-level fallback (wildcard).
      constexpr bool has_pattern_fallback =
          (core::traits::is_pattern_fallback_v<
               core::traits::case_pattern_t<std::decay_t<Cases>>>
           || ...);
      static_assert(has_pattern_fallback,
                    "[Patternia.match]: missing wildcard fallback. "
                    "Tip: add '__ >> handler' or '_ >> handler' as "
                    "the last case.");

      // Dummy fallback (unreachable due to wildcard).
      auto dummy_fallback =
          [](auto &&...) -> core::traits::detail::unreachable_t {
        return {};
      };
      using dummy_handler_t = decltype(dummy_fallback);

      // Validate the overall match expression.
      core::common::static_assert_valid_match<
          subject_type,
          dummy_handler_t,
          std::decay_t<Cases>...>();

      // Compute result type.
      using result_type = core::traits::match_result_t<
          subject_type,
          dummy_handler_t,
          std::decay_t<Cases>...>;

      // Build the cases tuple.
      auto cases_tuple = std::make_tuple(
          std::forward<Cases>(cases)...);

      // Evaluate.
      if constexpr (core::traits::is_void_like_v<result_type>) {
        core::engine::detail::match_impl::eval<result_type>(
            subject,
            std::move(cases_tuple),
            std::move(dummy_fallback));
      }
      else {
        return core::engine::detail::match_impl::eval<result_type>(
            subject,
            std::move(cases_tuple),
            std::move(dummy_fallback));
      }
    }

  } // namespace detail

  // Unified entry for pattern matching with inline cases.
  //
  // This is the recommended entry point for 0.9+ API. Cases are
  // provided directly as arguments as an alternative to the pipeline
  // form.
  //
  // Usage:
  //   match(value,
  //       lit(1) >> 100,
  //       lit(2) >> 200,
  //       __ >> 0);
  //
  // Requirements:
  //   - At least one case must be provided.
  //   - The last case must be a wildcard fallback (__ or _).
  //   - All handler return types must be compatible.
  template <typename T, typename Case0, typename... CasesRest>
  constexpr decltype(auto)
  match(T &value, Case0 &&case0, CasesRest &&...rest) {
    return detail::eval_match_cases(
        value,
        std::forward<Case0>(case0),
        std::forward<CasesRest>(rest)...);
  }

  // Entry for pipeline matching.
  //
  // Subject type is automatically deduced as `T&`.
  // Usage:
  //   - match(value) | on(case1, case2, ..., __ >> fallback)
  template <typename T>
  constexpr auto match(T &value) {
    using V = T &;
    return core::engine::detail::pipeline_match_context<V>::create(V(value));
  }

  // Caches an `on(...)` matcher behind a stateless factory.
  //
  // This is an explicit wrapper around the common handwritten
  // pattern: `static auto cases = on(...);`.
  //
  // Usage:
  //   - match(value) | static_on([] { return on(..., __ >>
  //   fallback); })
  //
  // The factory must be stateless so caching does not silently
  // freeze runtime captures on the first call.
  template <typename Factory>
  inline decltype(auto) static_on(Factory &&factory) {
    using factory_t = std::decay_t<Factory>;
    static_assert(
        std::is_empty_v<factory_t>,
        "[Patternia.static_on]: factory must be stateless. "
        "Tip: pass [] { return on(...); }.");
    static_assert(
        std::is_invocable_v<factory_t &>,
        "[Patternia.static_on]: factory must be invocable with no "
        "arguments.");

    using cases_t = std::decay_t<std::invoke_result_t<factory_t &>>;
    static_assert(
        core::dsl::detail::is_on_v<cases_t>,
        "[Patternia.static_on]: factory must return on(...).");

    static cases_t cases = std::forward<Factory>(factory)();
    return (cases);
  }

} // namespace ptn
