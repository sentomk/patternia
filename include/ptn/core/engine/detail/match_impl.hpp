#pragma once

// Core evaluation logic for a sequence of cases.
//
// This header contains the match implementation that delegates to the common
// evaluation logic for pattern matching.

#include <utility>

#include "ptn/core/common/eval.hpp"

namespace ptn::core::engine::detail {

  struct match_impl {

    // Evaluate the tuple of cases against a subject.
    // Delegates to the common evaluation logic for actual pattern matching.
    template <typename TV, typename CasesTuple, typename Otherwise>
    static constexpr decltype(auto)
    eval(TV &subject, CasesTuple &cases, Otherwise &&otherwise_handler) {
      return ptn::core::common::eval_cases(
          subject, cases, std::forward<Otherwise>(otherwise_handler));
    }
  };

} // namespace ptn::core::engine::detail
