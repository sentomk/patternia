#pragma once

#include <tuple>
#include <type_traits>
#include <utility>
#include "ptn/core/match_builder.hpp" // core API
#include "ptn/config.hpp"

/**
 * @defgroup main Main API
 * @brief Core public interface of Patternia.
 *
 * @file patternia.hpp
 * @brief Public entry header for Patternia
 * @details
 * This header serves as the unified interface for end users.
 * It includes the core DSL (`match_builder`) and all enabled pattern modules.
 * ### Example
 * @code {.cpp}
 * using namespace ptn;
 * auto res = match(cmd)
 *     .when(ci_value("start") >> "starting...")
 *     .when(ci_value("stop")  >> "stopping...")
 *     .otherwise("unkown");
 * @endcode
 *
 * @ingroup main
 * @version 0.4.2
 * @date 2025-11-11
 * @author sentomk (sentomk040924@gmail.com)
 * @copyright Copyright (c) 2025
 *
 */

#if PTN_ENABLE_VALUE_PATTERN
// clang-format off
#  include <ptn/patterns/value.hpp>
// clang-format on
#endif

#if PTN_ENABLE_RELATIONAL_PATTERN
// clang-format off
#include <ptn/patterns/relational.hpp>
// clang-format on
#endif

#if PTN_ENABLE_PREDICATE_PATTERN
// clang-format off
#include <ptn/patterns/predicate.hpp>
// clang-format on
#endif

/**
 * @namespace ptn
 * @brief Root namespace for Patternia.
 * @details
 * Contains all top-level APIs:
 * - `match()` — entry point to create a matching builder.
 * - DSL operators and combinators (`>>`, `&&`, `||`, etc.).
 * - Built-in pattern modules under `ptn::patterns`.
 * @see ptn::core::match_builder
 * @ingroup main
 */
namespace ptn {

  /**
   * @fn template <typename T> constexpr auto ptn::match(T &&value)
   * @brief Creates a match builder for the given subject.
   * @tparam T The type of the subject to match.
   * @param value The value to match against patterns.
   * @return A `match_builder` instance initialized with the given value.
   */
  template <typename T>
  constexpr auto match(T &&value) noexcept(
      std::is_nothrow_constructible_v<std::decay_t<T>, T &&>) {
    using V = std::decay_t<T>;
    return core::match_builder<V>::create(
        V(std::forward<T>(value)), std::tuple<>{});
  }

  /// @brief Import all submodules for convenient user access.
  using namespace core;          ///< Core match builder and traits.
  using namespace dsl;           ///< DSL syntax utilities (e.g. `>>`).
  using namespace patterns;      ///< Built-in pattern categories.
  using namespace patterns::ops; ///< Operators (`&&`, `||`, `!` etc.).

}; // namespace ptn