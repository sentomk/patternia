// IWYU pragma: begin_exports

#pragma once

// The single-entry-point header for the Patternia library.
//
// Include this file to get access to all of Patternia's core pattern matching
// capabilities, including value, type, and structural patterns, as well as
// the DSL operators and core matching functions.
//
// This is the recommended way to use Patternia.
//
// @namespace ptn

// --- Core Framework ---
// The fundamental CRTP base and traits.
#include "ptn/pattern/base/pattern_base.hpp"
#include "ptn/pattern/base/pattern_traits.hpp"
#include "ptn/pattern/base/pattern_kind.hpp"

// --- Core Matching Logic ---
// The main entry point functions
#include "ptn/core/engine/match.hpp"

// --- DSL Operators ---
// Enables the use of `&&`, `||`, `!` for pattern composition.
#include "ptn/core/dsl/ops.hpp"

// --- All Pattern Modules ---
// Includes all public pattern factories.
#include "ptn/pattern/value.hpp" // lit, pred, lt, eq, between, ...
#include "ptn/pattern/type.hpp"

namespace ptn {
  // Import DSL operators
  using ptn::core::dsl::ops::operator>>;
  using ptn::core::dsl::ops::operator&&;
  using ptn::core::dsl::ops::operator||;
  using ptn::core::dsl::ops::operator!;

  // Import value pattern factories
  using ptn::pat::value::between;
  using ptn::pat::value::eq;
  using ptn::pat::value::ge;
  using ptn::pat::value::gt;
  using ptn::pat::value::le;
  using ptn::pat::value::lit;
  using ptn::pat::value::lit_ci;
  using ptn::pat::value::lt;
  using ptn::pat::value::ne;
  using ptn::pat::value::pred;

  // Import type pattern factories
  using ptn::pat::type::from;
  using ptn::pat::type::in;
  using ptn::pat::type::is;
  using ptn::pat::type::not_in;

  // Create ptn::type namespace for direct access
  namespace type {
    using ptn::pat::type::from;
    using ptn::pat::type::in;
    using ptn::pat::type::is;
    using ptn::pat::type::not_in;
  } // namespace type

  // Create ptn::value namespace for direct access
  namespace value {
    using ptn::pat::value::between;
    using ptn::pat::value::eq;
    using ptn::pat::value::ge;
    using ptn::pat::value::gt;
    using ptn::pat::value::le;
    using ptn::pat::value::lit;
    using ptn::pat::value::lit_ci;
    using ptn::pat::value::lt;
    using ptn::pat::value::ne;
    using ptn::pat::value::pred;
  } // namespace value
} // namespace ptn

// IWYU pragma: end_exports
