// IWYU pragma: begin_exports

#pragma once

/**
 * @file patternia.hpp
 * @brief The single-entry-point header for the Patternia library.
 *
 * Include this file to get access to all of Patternia's core pattern matching
 * capabilities, including value, type, and structural patterns, as well as
 * the DSL operators and core matching functions.
 *
 * This is the recommended way to use Patternia.
 *
 * @namespace ptn
 */

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
  using ptn::core::dsl::ops::operator>>;
  using ptn::core::dsl::ops::operator&&;
  using ptn::core::dsl::ops::operator||;
  using ptn::core::dsl::ops::operator!;
} // namespace ptn

// IWYU pragma: end_exports
