#pragma once

// Common evaluation logic for matching and handler invocation.

#include <tuple>
#include <type_traits>
#include <utility>
#include <functional>

#include "ptn/config.hpp"
#include "ptn/core/common/common_traits.hpp"

namespace ptn::core::common {

  // SFINAE Helper for C++17
  namespace detail {
    // A helper to provide a `void` type for SFINAE.
    template <typename... Ts>
    struct void_type {
      using type = void;
    };

    // Detects if `p.match(s)` is a valid expression.
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

    // Detects if `p.bind(s)` is a valid expression.
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

  // Checks if a case's pattern matches a subject.
  template <typename Case, typename Subject>
  struct case_matcher {
    using case_type    = Case;
    using subject_type = Subject;
    using pattern_type = case_pattern_t<Case>;

    // Returns true if the pattern matches the subject.
    static constexpr bool
    matches(const case_type &c, const subject_type &subject) {
      // Supports both class-based patterns with a .match() method and
      // functional patterns (via std::invoke).
#if PTN_USE_CONCEPTS
      if constexpr (requires(const pattern_type &p, const subject_type &s) {
                      { p.match(s) } -> std::convertible_to<bool>;
                    }) {
        return c.pattern.match(subject);
      }
      else {
        return static_cast<bool>(std::invoke(c.pattern, subject));
      }
#else
      // Use a robust SFINAE check instead of the faulty member function
      // pointer check.
      if constexpr (detail::has_match_member_v<pattern_type, subject_type>) {
        return c.pattern.match(subject);
      }
      else {
        return static_cast<bool>(std::invoke(c.pattern, subject));
      }
#endif
    }
  };

  // Handler Invocation Logic

  // Invokes a handler with the subject and values bound by its pattern.
  template <typename Case, typename Subject>
  constexpr decltype(auto) invoke_handler(const Case &c, Subject &&subject) {
    using pattern_type = case_pattern_t<Case>;

// The check is different for C++17 and C++20.
#if PTN_USE_CONCEPTS
    static_assert(
        requires(const pattern_type &p, Subject &&s) {
          { p.bind(std::forward<Subject>(s)) };
        },
        "Pattern must have a 'bind(subject)' method that returns a tuple of "
        "bound values.");
#else
    // Use the robust SFINAE check instead of the faulty sizeof trick.
    static_assert(
        detail::has_bind_member_v<pattern_type, Subject>,
        "Pattern must have a 'bind(subject)' method that returns a tuple of "
        "bound values.");
#endif

    // Let the pattern perform the binding.
    auto bound_values = c.pattern.bind(std::forward<Subject>(subject));

    // Combine the subject and bound values into a single tuple.
    auto full_args = std::tuple_cat(
        std::forward_as_tuple(std::forward<Subject>(subject)),
        std::move(bound_values));

    // Apply the handler to the full set of arguments.
    return std::apply(c.handler, std::move(full_args));
  }

  // Case Sequence Evaluation
  namespace detail {
    // Recursive implementation for evaluating a tuple of cases.
    template <
        std::size_t I,
        typename Subject,
        typename CasesTuple,
        typename Otherwise,
        std::size_t N = std::tuple_size_v<std::remove_reference_t<CasesTuple>>>
    constexpr decltype(auto) eval_cases_impl(
        Subject &subject, CasesTuple &cases, Otherwise &&otherwise_handler) {
      if constexpr (I >= N) {
        // Base case: no case matched, invoke the `otherwise` handler.
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
          // Found a match, invoke its handler.
          return invoke_handler(current_case, std::forward<Subject>(subject));
        }
        else {
          // Try the next case.
          return eval_cases_impl<I + 1>(
              subject, cases, std::forward<Otherwise>(otherwise_handler));
        }
      }
    }
  } // namespace detail

  // Public entry point to evaluate a sequence of cases.
  template <typename Subject, typename CasesTuple, typename Otherwise>
  constexpr decltype(auto) eval_cases(
      Subject &subject, CasesTuple &cases, Otherwise &&otherwise_handler) {
    using tuple_t           = std::remove_reference_t<CasesTuple>;
    constexpr std::size_t N = std::tuple_size_v<tuple_t>;

    return detail::eval_cases_impl<0>(
        subject, cases, std::forward<Otherwise>(otherwise_handler));
  }

} // namespace ptn::core::common
