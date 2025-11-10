#pragma once

/**
 * @file patternia.hpp
 * @brief Public entry header for Patternia — the modern C++ pattern matching
 * DSL.
 * @ingroup main
 *
 * This header serves as the unified interface for end users.
 * It includes the core DSL (`match_builder`) and all enabled pattern modules
 * (value, relational, predicate, etc.), providing a fluent interface:
 *
 * @code{.cpp}
 * using namespace ptn;
 * auto res = match(cmd)
 *     .when(ci_value("start") >> [] { return "starting"; })
 *     .when(ci_value("stop")  >> [] { return "stopping"; })
 *     .otherwise("unknown");
 * @endcode
 */

#include <tuple>
#include <type_traits>
#include <utility>
#include "ptn/core/match_builder.hpp" // core API
#include "ptn/config.hpp"

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

namespace ptn {

  /**
   * @defgroup main Patternia Entry Point
   * @brief Core entry interface of Patternia DSL.
   * @{
   */

  /**
   * @brief Start a pattern-matching expression.
   * @tparam T The type of the subject value.
   * @param value The value to match against patterns.
   * @return A `core::match_builder` ready for `.when()` and `.otherwise()`
   * chaining.
   *
   * This is the only public entry to the Patternia DSL.
   * It constructs a `match_builder` with the given value, enabling a fluent
   * interface:
   *
   * @code{.cpp}
   * match(5)
   *   .when(gt(0) >> []{ return "positive"; })
   *   .when(lt(0) >> []{ return "negative"; })
   *   .otherwise("zero");
   * @endcode
   */

  /* wrapper: the only public entry to start a match chain */
  template <typename T>
  constexpr auto match(T &&value) noexcept(
      std::is_nothrow_constructible_v<std::decay_t<T>, T &&>) {
    using V = std::decay_t<T>;
    return core::match_builder<V>::create(
        V(std::forward<T>(value)), std::tuple<>{});
  }
  /** @} */ // end of main

  using namespace core;
  using namespace dsl;
  using namespace patterns;
  using namespace patterns::ops;

}; // namespace ptn