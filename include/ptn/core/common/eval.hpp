#pragma once

/**
 * @file eval.hpp
 * @brief Common evaluation logic for matching and handler invocation.
 */

#include <tuple>
#include <type_traits>
#include <utility>

#include "ptn/config.hpp"
#include "ptn/core/common/common_traits.hpp"

namespace ptn::core::common {

  /** Case Matching */

  /// @brief Checks if a case's pattern matches a subject.
  template <typename Case, typename Subject>
  struct case_matcher {
    using case_type    = Case;
    using subject_type = Subject;
    using pattern_type = case_pattern_t<Case>;

    /// @brief Returns true if the pattern matches the subject.
    static constexpr bool
    matches(const case_type &c, const subject_type &subject) {
      // Supports both class-based patterns with a .match() method and
      // functional patterns.
      if constexpr (std::is_member_function_pointer_v<
                        decltype(&pattern_type::match)>) {
        return c.pattern.match(subject);
      }
      else {
        return static_cast<bool>(std::invoke(c.pattern, subject));
      }
    }
  };

  /** Handler Invocation Logic */

  /// @brief Invokes a handler with values bound by its pattern.
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
    // C++17 SFINAE trick to check if `bind` is a valid expression.
    static_assert(
        sizeof(
            decltype(std::declval<const pattern_type &>().bind(std::declval<Subject>()), void())) !=
            0,
        "Pattern must have a 'bind(subject)' method that returns a tuple of "
        "bound values.");
#endif

    // Let the pattern perform the binding.
    auto bound_values = c.pattern.bind(std::forward<Subject>(subject));

    // Apply the handler to the bound values.
    return std::apply(c.handler, std::move(bound_values));
  }

  /** Case Sequence Evaluation */
  namespace detail {
    /// @brief Recursive implementation for evaluating a tuple of cases.
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

  /// @brief Public entry point to evaluate a sequence of cases.
  template <typename Subject, typename CasesTuple, typename Otherwise>
  constexpr decltype(auto) eval_cases(
      Subject &subject, CasesTuple &cases, Otherwise &&otherwise_handler) {
    using tuple_t           = std::remove_reference_t<CasesTuple>;
    constexpr std::size_t N = std::tuple_size_v<tuple_t>;

    return detail::eval_cases_impl<0>(
        subject, cases, std::forward<Otherwise>(otherwise_handler));
  }

} // namespace ptn::core::common
