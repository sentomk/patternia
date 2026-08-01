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
#include "ptn/pattern/lit.hpp"             // lit, val, lit_ci
#include "ptn/pattern/bind.hpp"            // $, $(...)
#include "ptn/pattern/wildcard.hpp"        // wildcard
#include "ptn/pattern/modifiers/guard.hpp" // guard
#include "ptn/pattern/structural.hpp"      // has
#include "ptn/pattern/combinator.hpp"      // any/all
#include "ptn/pattern/type.hpp"            // is<T>, alt<I>
#include "ptn/pattern/pred.hpp"            // pred
#include "ptn/pattern/negation.hpp"        // neg

namespace ptn {
  // Imports DSL operators.
  using ptn::core::dsl::ops::operator>>;
  using ptn::core::dsl::detail::on;

  using ptn::pat::lit;
  using ptn::pat::lit_ci;
  using ptn::pat::val;

  using ptn::pat::$;

  using ptn::pat::_;

  // Guard utilities.
  using ptn::pat::mod::operator&&;
  using ptn::pat::mod::operator||;
  using ptn::pat::mod::rng;

  // Structural matching utilities.
  using ptn::pat::_ign;
  using ptn::pat::all;
  using ptn::pat::any;
  using ptn::pat::has;

  // Type-pattern utilities (variable templates).
  using ptn::pat::alt;
  using ptn::pat::is;

  // Predicate pattern utility.
  using ptn::pat::pred;

  // Negation pattern utility.
  using ptn::pat::neg;

} // namespace ptn

// Optional sugar for the statically cached `on(...)` factory form.
//
// Expands to an immediately-invoked lambda that caches the matcher
// in a function-local static. This avoids both the matcher
// construction cost on every evaluation and the function-call
// boundary of `static_on`.
#ifndef PTN_ON
#define PTN_ON(...)                                                 \
  ([]() -> auto & {                                                 \
    static auto _ptn_cases = ::ptn::on(__VA_ARGS__);                \
    return _ptn_cases;                                              \
  }())
#endif

// PTN_BIND(Type, member0, member1, ...)
//
// Declares member-anchored named placeholders for use in guard
// expressions attached to has<...>. Each name must designate a
// non-static data member of Type: the macro expands name to
// constexpr member_t<&Type::name>. Declarations are valid at
// namespace or block scope.
//
// Inside a guard, names resolve to the position of their member
// in the has<...> member list at compile time. They follow
// members, not positions, so the order of member pointers in
// has<...> does not matter.
//
// Example:
//   struct Point { int x; int y; };
//   PTN_BIND(Point, x, y);
//
//   match(p) | PTN_ON(
//       $(has<&Point::x, &Point::y>)[x*x + y*y == 25]
//           >> [](auto& p) { return dist(p); }
//   );
//
// A misspelled member name fails at the PTN_BIND line, and a name
// used with a has<...> that does not list its member fails a
// static_assert.
//
// Supports 1 to 10 member names.
//
// Implementation notes:
// - PTN_BIND_DECL declares a single named placeholder.
// - PTN_BIND_N is defined by chaining: it expands to
//   PTN_BIND_{N-1} plus one more declaration, so adding a new
//   arity only costs one short macro instead of a full rewrite.
// The static storage duration lets block-scope declarations appear
// inside PTN_ON's captureless caching lambda: only automatic
// variables require captures; statics do not.
#define PTN_BIND_DECL(Type, name)                                   \
  static constexpr ::ptn::pat::mod::member_t<&Type::name> name{};

#define PTN_BIND_1(Type, m0) PTN_BIND_DECL(Type, m0)

#define PTN_BIND_2(Type, m0, m1)                                    \
  PTN_BIND_EXPAND(PTN_BIND_1(Type, m0))                             \
  PTN_BIND_DECL(Type, m1)

#define PTN_BIND_3(Type, m0, m1, m2)                                \
  PTN_BIND_EXPAND(PTN_BIND_2(Type, m0, m1))                         \
  PTN_BIND_DECL(Type, m2)

#define PTN_BIND_4(Type, m0, m1, m2, m3)                            \
  PTN_BIND_EXPAND(PTN_BIND_3(Type, m0, m1, m2))                     \
  PTN_BIND_DECL(Type, m3)

#define PTN_BIND_5(Type, m0, m1, m2, m3, m4)                        \
  PTN_BIND_EXPAND(PTN_BIND_4(Type, m0, m1, m2, m3))                 \
  PTN_BIND_DECL(Type, m4)

#define PTN_BIND_6(Type, m0, m1, m2, m3, m4, m5)                    \
  PTN_BIND_EXPAND(PTN_BIND_5(Type, m0, m1, m2, m3, m4))             \
  PTN_BIND_DECL(Type, m5)

#define PTN_BIND_7(Type, m0, m1, m2, m3, m4, m5, m6)                \
  PTN_BIND_EXPAND(PTN_BIND_6(Type, m0, m1, m2, m3, m4, m5))         \
  PTN_BIND_DECL(Type, m6)

#define PTN_BIND_8(Type, m0, m1, m2, m3, m4, m5, m6, m7)            \
  PTN_BIND_EXPAND(PTN_BIND_7(Type, m0, m1, m2, m3, m4, m5, m6))     \
  PTN_BIND_DECL(Type, m7)

#define PTN_BIND_9(Type, m0, m1, m2, m3, m4, m5, m6, m7, m8)        \
  PTN_BIND_EXPAND(PTN_BIND_8(Type, m0, m1, m2, m3, m4, m5, m6, m7)) \
  PTN_BIND_DECL(Type, m8)

#define PTN_BIND_10(Type, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9)   \
  PTN_BIND_EXPAND(                                                  \
      PTN_BIND_9(Type, m0, m1, m2, m3, m4, m5, m6, m7, m8))         \
  PTN_BIND_DECL(Type, m9)

#define PTN_BIND_PICK(                                              \
    _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, NAME, ...)             \
  NAME

// Extra indirection layers for MSVC traditional preprocessor
// compatibility. Without these, MSVC treats __VA_ARGS__ as a single
// token when forwarding between macros, breaking the count dispatch
// and the final macro invocation.
#define PTN_BIND_EXPAND(...) __VA_ARGS__
#define PTN_BIND_DISPATCH(Macro, Type, ...)                         \
  PTN_BIND_EXPAND(Macro(Type, __VA_ARGS__))

#ifndef PTN_BIND
#define PTN_BIND(Type, ...)                                         \
  PTN_BIND_DISPATCH(PTN_BIND_EXPAND(PTN_BIND_PICK(__VA_ARGS__,      \
                                                  PTN_BIND_10,      \
                                                  PTN_BIND_9,       \
                                                  PTN_BIND_8,       \
                                                  PTN_BIND_7,       \
                                                  PTN_BIND_6,       \
                                                  PTN_BIND_5,       \
                                                  PTN_BIND_4,       \
                                                  PTN_BIND_3,       \
                                                  PTN_BIND_2,       \
                                                  PTN_BIND_1)),     \
                    Type,                                           \
                    __VA_ARGS__)
#endif

// IWYU pragma: end_exports
