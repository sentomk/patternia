#pragma once
/**
 * @file match_impl.hpp
 * @brief Core evaluation logic for a sequence of cases.
 */

#include <utility>

#include "ptn/core/common/eval.hpp"

namespace ptn::core::engine::detail {

  struct match_impl {

    /**
     * @brief Evaluate the tuple of cases against a subject.
     */
    template <typename TV, typename CasesTuple, typename Otherwise>
    static constexpr decltype(auto)
    eval(TV &subject, CasesTuple &cases, Otherwise &&otherwise_handler) {
      return ptn::core::common::eval_cases(
          subject, cases, std::forward<Otherwise>(otherwise_handler));
    }
  };

} // namespace ptn::core::engine::detail
