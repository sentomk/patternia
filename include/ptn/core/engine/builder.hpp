#pragma once

// Public-facing builder type for the Core matching engine.
//
// This header provides the public interface for Patternia's fluent builder
// pattern that enables chaining .when() clauses in pattern matching
// expressions.
//
// Internally, it is an alias to `ptn::core::engine::detail::match_builder`.

// Pull in the implementation
#include "ptn/core/engine/detail/builder_impl.hpp"

namespace ptn::core {

  // Public alias forwarding to the actual implementation.
  // This type enables the fluent builder pattern for pattern matching:
  //   match(value).when(case1).when(case2).otherwise(default_handler)
  //
  // TV: Subject type being matched
  // Cases: Type representing the sequence of case expressions
  template <typename TV, typename Cases>
  using match_builder = engine::detail::match_builder<TV, false, Cases>;

} // namespace ptn::core
