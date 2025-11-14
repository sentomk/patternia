#pragma once
/**
 * @file patternia.hpp
 * @brief Public entry header for Patternia — the modern C++ pattern matching
 * DSL.
 *
 * Provides a unified interface for end users. It pulls in:
 *   - Core Layer (`match_builder`)
 *   - Pattern Layer (value, relational, predicate)
 *   - Type Layer (optional)
 *   - DSL operators: >>, &&, ||, !
 *
 */

#include <tuple>
#include <type_traits>
#include <utility>

#include "ptn/config.hpp"
#include "ptn/core/match_builder.hpp"
#include "ptn/core/dsl/case_expr.hpp"
#include "ptn/core/dsl/operators.hpp"

// -------- Pattern Layer --------
#if PTN_ENABLE_VALUE_PATTERN
#include "ptn/pattern/value/literal.hpp"
#endif

#if PTN_ENABLE_RELATIONAL_PATTERN
#include "ptn/pattern/value/relational.hpp"
#endif

#if PTN_ENABLE_PREDICATE_PATTERN
#include "ptn/pattern/value/predicate.hpp"
#endif

// -------- Type Layer --------
#if PTN_ENABLE_TYPE
#include "ptn/pattern/type/type.hpp"
#include "ptn/pattern/type/type_of.hpp"
#endif

/**
 * @namespace ptn
 * @brief Root namespace for Patternia — all public APIs live here.
 *
 * Contains:
 *   - `match()` — entry point to pattern matching
 *   - Pattern Layer (value/relational/predicate)
 *   - Type Layer (enabled optionally)
 *   - DSL namespace (`ptn::dsl` and `ptn::dsl::ops`)
 *
 * The DSL operators are intentionally *not* injected into `ptn`
 * to avoid namespace pollution. Users should explicitly:
 *
 */
namespace ptn {

  /// @brief Entry function for the pattern matching DSL.
  template <typename T>
  constexpr auto match(T &&value) noexcept(
      std::is_nothrow_constructible_v<std::decay_t<T>, T &&>) {

    using V = std::decay_t<T>;
    return core::match_builder<V>::create(
        V(std::forward<T>(value)), std::tuple<>{});
  }

  // ---- Re-export public namespaces ----
  using namespace core;           // match_builder, etc.
  using namespace dsl;            // case_expr, DSL constructs
  using namespace pattern::value; // literal / relational / predicate patterns
  using namespace ptn::dsl::ops;  // all DSL operators: >> && || !

  // ---- Export Type Layer (ONLY selected symbols) ----
#if PTN_ENABLE_TYPE
  using pattern::type::type_is;
  using pattern::type::type_of;
  using pattern::type::type_tag;
#endif

} // namespace ptn
