#pragma once
/**
 * @file match_impl.hpp
 * @brief Core evaluation logic for a sequence of cases.
 */

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

#include "ptn/config.hpp"
#include "ptn/core/engine/detail/case_eval.hpp"

namespace ptn::core::detail {

  struct match_impl {
    /**
     * @brief Evaluate the tuple of cases against a subject.
     *
     * TV          : subject type
     * CasesTuple  : std::tuple<Cases...>
     * Otherwise   : fallback handler
     */
    template <typename TV, typename CasesTuple, typename Otherwise>
    static constexpr decltype(auto)
    eval(TV &subject, CasesTuple &cases, Otherwise &&otherwise_handler) {
      using tuple_type        = std::remove_reference_t<CasesTuple>;
      constexpr std::size_t N = std::tuple_size_v<tuple_type>;

      return eval_chain<0>(
          subject,
          cases,
          std::forward<Otherwise>(otherwise_handler),
          std::integral_constant<std::size_t, N>{});
    }

  private:
    // Recursive chain over tuple indices [0, N)
    template <
        std::size_t I,
        typename TV,
        typename CasesTuple,
        typename Otherwise>
    static constexpr decltype(auto) eval_chain(
        TV         &subject,
        CasesTuple &cases,
        Otherwise &&otherwise_handler,
        std::integral_constant<std::size_t, I>) {
      using tuple_type        = std::remove_reference_t<CasesTuple>;
      constexpr std::size_t N = std::tuple_size_v<tuple_type>;

      if constexpr (I >= N) {
        // No case matched; fall back to "otherwise"
        if constexpr (std::is_invocable_v<Otherwise, TV &>) {
          return std::forward<Otherwise>(otherwise_handler)(subject);
        }
        else {
          return std::forward<Otherwise>(otherwise_handler)();
        }
      }
      else {
        auto &c         = std::get<I>(cases);
        using case_type = std::remove_reference_t<decltype(c)>;
        using eval_t    = case_eval<case_type, TV>;

        if (eval_t::matches(c, subject)) {
          // First matching case — invoke its handler
          return eval_t::invoke(c, subject);
        }
        else {
          // Try next
          return eval_chain<I + 1>(
              subject,
              cases,
              std::forward<Otherwise>(otherwise_handler),
              std::integral_constant<std::size_t, N>{});
        }
      }
    }
  };

} // namespace ptn::core::detail
