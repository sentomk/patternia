#pragma once

// Optimization utilities for pattern matching case sequences.
//
// This header provides optimization hooks for improving pattern matching performance
// through case reordering, elimination of unreachable cases, and other compile-time
// optimizations. Currently serves as a placeholder for future optimization strategies.

namespace ptn::core::common {

  namespace detail {
    /// Primary template for case sequence optimization
    /// Currently provides identity transformation - future implementations
    /// may include case reordering, dead code elimination, etc.
    template <typename Subject, typename CasesTuple>
    struct optimize_case_sequence {
      using type = CasesTuple;
    };
  } // namespace detail

  /// Convenience alias for optimized case sequence type
  /// Applies optimization transformations to a case sequence
  template <typename Subject, typename CasesTuple>
  using optimize_case_sequence_t =
      typename detail::optimize_case_sequence<Subject, CasesTuple>::type;

} // namespace ptn::core::common
