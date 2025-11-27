#pragma once
/**
 * @file case_eval.hpp
 * @brief Low-level evaluation of a single case (pattern + handler).
 */

#include <functional>
#include <type_traits>
#include <utility>

#include "ptn/config.hpp"

namespace ptn::core::detail {

  // Helper for static_assert that depends on a template parameter
  template <typename>
  struct dependent_false : std::false_type {};

  template <typename Case, typename Subject>
  struct case_eval {
    using case_type    = Case;
    using subject_type = Subject;

    // Use ADL-friendly pattern type deduction if needed
    using pattern_type = decltype(std::declval<const Case &>().pattern);
    using handler_type = decltype(std::declval<const Case &>().handler);

    /**
     * @brief Check whether this case matches the subject.
     *
     * Default policy:
     *   - call `pattern(subject)` via std::invoke
     */
    static constexpr bool
    matches(const case_type &c, const subject_type &subject) {
      if constexpr (std::is_invocable_r_v<bool, pattern_type, subject_type>) {
        return std::invoke(c.pattern, subject);
      }
      else if constexpr (std::is_invocable_v<pattern_type, subject_type>) {
        // Allow non-bool results, but treat them as "truthy"
        return static_cast<bool>(std::invoke(c.pattern, subject));
      }
      else {
        static_assert(
            dependent_false<Case>::value,
            "[case_eval]: Pattern must be invocable as "
            "bool(pattern(subject)).");
      }
    }

    /**
     * @brief Invoke the handler associated with this case.
     *
     * Handler policy:
     *   - Prefer handler(subject) if available
     *   - Otherwise, call handler()
     */
    template <typename S>
    static constexpr decltype(auto) invoke(const case_type &c, S &&subject) {
      using handler_t = handler_type;

      if constexpr (std::is_invocable_v<handler_t, S &&>) {
        return std::invoke(c.handler, std::forward<S>(subject));
      }
      else if constexpr (std::is_invocable_v<handler_t>) {
        return std::invoke(c.handler);
      }
      else {
        static_assert(
            dependent_false<Case>::value,
            "[case_eval]: Handler must be invocable either as handler(subject) "
            "or handler().");
      }
    }
  };

} // namespace ptn::core::detail
