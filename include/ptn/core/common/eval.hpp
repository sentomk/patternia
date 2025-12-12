#pragma once

// Common evaluation logic for matching and handler invocation.
//
// This header provides the core runtime evaluation machinery for pattern
// matching, including case matching logic, handler invocation with bound
// values, and sequential case evaluation.

#include <tuple>
#include <type_traits>
#include <utility>
#include <functional>

#include "ptn/core/common/common_traits.hpp"

namespace ptn::core::common {

  // SFINAE Helper for C++17
  namespace detail {
    // Helper type to provide void type for SFINAE expressions
    template <typename... Ts>
    struct void_type {
      using type = void;
    };

    // Detects if a pattern has a .match(subject) member function
    template <typename P, typename S, typename = void>
    struct has_match_member : std::false_type {};

    template <typename P, typename S>
    struct has_match_member<
        P,
        S,
        typename void_type<decltype(std::declval<P>().match(
            std::declval<S>()))>::type> : std::true_type {};

    template <typename P, typename S>
    constexpr bool has_match_member_v = has_match_member<P, S>::value;

    // Detects if a pattern has a .bind(subject) member function
    template <typename P, typename S, typename = void>
    struct has_bind_member : std::false_type {};

    template <typename P, typename S>
    struct has_bind_member<
        P,
        S,
        typename void_type<decltype(std::declval<P>().bind(
            std::declval<S>()))>::type> : std::true_type {};

    template <typename P, typename S>
    constexpr bool has_bind_member_v = has_bind_member<P, S>::value;
  } // namespace detail

  // Case Matching

  // Checks if a case's pattern matches a subject value
  // Supports both class-based patterns with .match() and functional patterns
  template <typename Case, typename Subject>
  struct case_matcher {
    using case_type    = Case;
    using subject_type = Subject;
    using pattern_type = case_pattern_t<Case>;

    // Returns true if the pattern matches the subject
    static constexpr bool
    matches(const case_type &c, const subject_type &subject) {
      if constexpr (detail::has_match_member_v<pattern_type, subject_type>) {
        return c.pattern.match(subject);
      }
      else {
        return static_cast<bool>(std::invoke(c.pattern, subject));
      }
    }
  };

  // Handler Invocation Logic

  namespace detail {

    // MSVC-friendly tuple invocation
    // Avoids: std::apply + dependent lambda + if constexpr
    template <typename F, typename Tuple, std::size_t... I>
    constexpr decltype(auto)
    invoke_from_tuple_impl(F &&f, Tuple &&t, std::index_sequence<I...>) {

      using result_t = std::
          invoke_result_t<F, decltype(std::get<I>(std::forward<Tuple>(t)))...>;

      if constexpr (std::is_void_v<result_t>) {
        std::invoke(std::forward<F>(f), std::get<I>(std::forward<Tuple>(t))...);
        return;
      }
      else {
        return std::invoke(
            std::forward<F>(f), std::get<I>(std::forward<Tuple>(t))...);
      }
    }

    template <typename F, typename Tuple>
    constexpr decltype(auto) invoke_from_tuple(F &&f, Tuple &&t) {
      using tuple_t = std::remove_reference_t<Tuple>;
      return invoke_from_tuple_impl(
          std::forward<F>(f),
          std::forward<Tuple>(t),
          std::make_index_sequence<std::tuple_size_v<tuple_t>>{});
    }

  } // namespace detail

  // Invokes a handler with the values bound by its pattern
  //
  // Design A: handler receives only the bound values from pattern matching
  //   handler( pattern.bind(subject)... )
  //   The subject itself is not passed as an extra first argument
  template <typename Case, typename Subject>
  constexpr decltype(auto) invoke_handler(const Case &c, Subject &&subject) {
    using pattern_type = case_pattern_t<Case>;

    static_assert(
        detail::has_bind_member_v<pattern_type, Subject>,
        "Pattern must have a 'bind(subject)' method that returns a tuple of "
        "bound values.");

    // Extract bound values from the pattern
    auto bound_values = c.pattern.bind(std::forward<Subject>(subject));
    return detail::invoke_from_tuple(c.handler, bound_values);
  }

  // Case Sequence Evaluation
  namespace detail {
    // Recursive implementation for evaluating a tuple of cases in sequence
    template <
        std::size_t I,
        typename Subject,
        typename CasesTuple,
        typename Otherwise,
        std::size_t N = std::tuple_size_v<std::remove_reference_t<CasesTuple>>>
    constexpr decltype(auto) eval_cases_impl(
        Subject &subject, CasesTuple &cases, Otherwise &&otherwise_handler) {
      if constexpr (I >= N) {
        // Base case: no case matched, invoke the otherwise handler
        if constexpr (std::is_invocable_v<Otherwise, Subject &>) {
          return std::forward<Otherwise>(otherwise_handler)(subject);
        }
        else {
          return std::forward<Otherwise>(otherwise_handler)();
        }
      }
      else {
        auto &current_case = std::get<I>(cases);
        using case_t       = std::remove_reference_t<decltype(current_case)>;

        if (case_matcher<case_t, Subject>::matches(current_case, subject)) {
          // Found a match, invoke its handler
          return invoke_handler(current_case, std::forward<Subject>(subject));
        }
        else {
          // Try the next case
          return eval_cases_impl<I + 1>(
              subject, cases, std::forward<Otherwise>(otherwise_handler));
        }
      }
    }
  } // namespace detail

  // Public entry point to evaluate a sequence of cases against a subject
  // Tries each case in order, invoking the first matching handler
  template <typename Subject, typename CasesTuple, typename Otherwise>
  constexpr decltype(auto) eval_cases(
      Subject &subject, CasesTuple &cases, Otherwise &&otherwise_handler) {
    using tuple_t           = std::remove_reference_t<CasesTuple>;
    constexpr std::size_t N = std::tuple_size_v<tuple_t>;

    (void) N; // silence unused warning if needed

    return detail::eval_cases_impl<0>(
        subject, cases, std::forward<Otherwise>(otherwise_handler));
  }

} // namespace ptn::core::common
