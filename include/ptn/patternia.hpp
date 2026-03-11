// IWYU pragma: begin_exports

#pragma once

// The single-entry-point header for the Patternia library.
//
// Include this file to get access to all of Patternia's core pattern
// matching capabilities, including value, type, and structural
// patterns, as well as the DSL operators and core matching
// functions.
//
// This is the recommended way to use Patternia.

// --- Core Framework ---
// The fundamental CRTP base and traits.
#include "ptn/pattern/base/pattern_base.hpp"
#include "ptn/pattern/base/pattern_traits.hpp"
#include "ptn/pattern/base/pattern_kind.hpp"

// --- Core Matching Logic ---
// Provides the main entry-point functions.
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
#include "ptn/pattern/type.hpp" // is<T>, alt<I>

namespace ptn {
  // Imports DSL operators.
  using ptn::core::dsl::ops::operator>>;
  using ptn::core::dsl::detail::on;

  using ptn::pat::lit;
  using ptn::pat::lit_ci;

  using ptn::pat::$;

  using ptn::pat::_;
  using ptn::pat::__;

  // Guard utilities.
  using ptn::pat::mod::_0;
  using ptn::pat::mod::operator&&;
  using ptn::pat::mod::operator||;
  using ptn::pat::mod::arg;
  using ptn::pat::mod::rng;

  // Structural matching utilities.
  using ptn::pat::_ign;
  using ptn::pat::has;

  // Type-pattern utilities (variable templates).
  using ptn::pat::alt;
  using ptn::pat::is;

} // namespace ptn

// Optional sugar for the statically cached `on(...)` factory form.
#ifndef PTN_ON
#define PTN_ON(...)                                                 \
  (::ptn::static_on([] { return ::ptn::on(__VA_ARGS__); }))
#endif

#define PTN_DETAIL_WHERE_CAT_IMPL(a, b) a##b
#define PTN_DETAIL_WHERE_CAT(a, b) PTN_DETAIL_WHERE_CAT_IMPL(a, b)
#define PTN_DETAIL_WHERE_COUNT_IMPL(a1, a2, a3, a4, a5, n, ...) n
#define PTN_DETAIL_WHERE_COUNT(...)                                 \
  PTN_DETAIL_WHERE_COUNT_IMPL(__VA_ARGS__, 5, 4, 3, 2, 1)
#define PTN_DETAIL_WHERE_COUNT_TUPLE(args)                          \
  PTN_DETAIL_WHERE_COUNT args
#define PTN_DETAIL_WHERE_NAME_1(a, ...) a
#define PTN_DETAIL_WHERE_NAME_2(a, b, ...) b
#define PTN_DETAIL_WHERE_NAME_3(a, b, c, ...) c
#define PTN_DETAIL_WHERE_NAME_4(a, b, c, d, ...) d
#define PTN_DETAIL_WHERE_NAME_5(a, b, c, d, e, ...) e
#define PTN_DETAIL_WHERE_SELECT(n)                                  \
  PTN_DETAIL_WHERE_CAT(PTN_DETAIL_WHERE_, n)
#define PTN_DETAIL_WHERE_INVOKE(n, args, ...)                       \
  PTN_DETAIL_WHERE_SELECT(n)(args, __VA_ARGS__)

// Expands a named guard expression into a stateless predicate object.
// The tuple of bound values is mapped to lambda parameters by position.
// This macro currently supports 1 to 5 names.
#define PTN_DETAIL_WHERE_1(args, ...)                               \
  (::ptn::pat::mod::make_callable_guard(                            \
      [](auto &&PTN_DETAIL_WHERE_NAME_1 args) -> bool {             \
        return static_cast<bool>(__VA_ARGS__);                      \
      }))

#define PTN_DETAIL_WHERE_2(args, ...)                               \
  (::ptn::pat::mod::make_callable_guard(                            \
      [](auto &&PTN_DETAIL_WHERE_NAME_1 args,                       \
         auto &&PTN_DETAIL_WHERE_NAME_2 args) -> bool {             \
        return static_cast<bool>(__VA_ARGS__);                      \
      }))

#define PTN_DETAIL_WHERE_3(args, ...)                               \
  (::ptn::pat::mod::make_callable_guard(                            \
      [](auto &&PTN_DETAIL_WHERE_NAME_1 args,                       \
         auto &&PTN_DETAIL_WHERE_NAME_2 args,                       \
         auto &&PTN_DETAIL_WHERE_NAME_3 args) -> bool {             \
        return static_cast<bool>(__VA_ARGS__);                      \
      }))

#define PTN_DETAIL_WHERE_4(args, ...)                               \
  (::ptn::pat::mod::make_callable_guard(                            \
      [](auto &&PTN_DETAIL_WHERE_NAME_1 args,                       \
         auto &&PTN_DETAIL_WHERE_NAME_2 args,                       \
         auto &&PTN_DETAIL_WHERE_NAME_3 args,                       \
         auto &&PTN_DETAIL_WHERE_NAME_4 args) -> bool {             \
        return static_cast<bool>(__VA_ARGS__);                      \
      }))

#define PTN_DETAIL_WHERE_5(args, ...)                               \
  (::ptn::pat::mod::make_callable_guard(                            \
      [](auto &&PTN_DETAIL_WHERE_NAME_1 args,                       \
         auto &&PTN_DETAIL_WHERE_NAME_2 args,                       \
         auto &&PTN_DETAIL_WHERE_NAME_3 args,                       \
         auto &&PTN_DETAIL_WHERE_NAME_4 args,                       \
         auto &&PTN_DETAIL_WHERE_NAME_5 args) -> bool {             \
        return static_cast<bool>(__VA_ARGS__);                      \
      }))

#ifndef PTN_WHERE
// Creates a named guard predicate from 1 to 5 parameter names.
#define PTN_WHERE(args, ...)                                        \
  PTN_DETAIL_WHERE_INVOKE(                                          \
      PTN_DETAIL_WHERE_COUNT_TUPLE(args),                           \
      args,                                                         \
      __VA_ARGS__)
#endif

#ifndef PTN_LET
// Single-value shorthand for PTN_WHERE((name), expr).
#define PTN_LET(name, ...)                                          \
  PTN_WHERE((name), __VA_ARGS__)
#endif

// IWYU pragma: end_exports
