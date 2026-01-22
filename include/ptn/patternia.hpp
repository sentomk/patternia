// IWYU pragma: begin_exports

#pragma once

// The single-entry-point header for the Patternia library.
//
// Include this file to get access to all of Patternia's core pattern matching
// capabilities, including value, type, and structural patterns, as well as
// the DSL operators and core matching functions.
//
// This is the recommended way to use Patternia.

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
#include "ptn/pattern/lit.hpp"             // lit, lit_ci
#include "ptn/pattern/bind.hpp"            // bind
#include "ptn/pattern/wildcard.hpp"        // wildcard
#include "ptn/pattern/modifiers/guard.hpp" // guard
#include "ptn/pattern/structural.hpp"      // has
#include "ptn/pattern/type.hpp"            // type::is

namespace ptn {
  using ptn::core::engine::match;

  // Import DSL operators
  using ptn::core::dsl::ops::operator>>;
  using ptn::core::dsl::ops::cases;

  using ptn::pat::lit;
  using ptn::pat::lit_ci;

  using ptn::pat::bind;

  using ptn::pat::__;

  // guard
  using ptn::pat::mod::_;
  using ptn::pat::mod::operator&&;
  using ptn::pat::mod::operator||;
  using ptn::pat::mod::arg;
  using ptn::pat::mod::rng;

  // structural
  using ptn::pat::_ign;
  using ptn::pat::has;

  // type
  namespace type = ptn::pat::type;
  using ptn::pat::type::is;

} // namespace ptn

// IWYU pragma: end_exports
