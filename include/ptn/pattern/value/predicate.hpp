#pragma once

#include <string>
#include <utility>

#include "ptn/pattern/value/detail/predicate_pattern.hpp"
#include "ptn/config.hpp"

/**
 * @file predicate.hpp
 * @brief Public API for predicate-based patterns and logical composition.
 */

namespace ptn::pattern::value {

  /**
   * @brief Factory for predicate_pattern.
   *
   * Ensures:
   *   - F is callable
   *   - F(value) is convertible to bool
   */
  template <typename F>
  constexpr auto pred(F &&f) {
    using Fn = std::decay_t<F>;

    // The actual type checking will happen during pattern matching
    static_assert(
        std::is_invocable_v<Fn, int> || std::is_invocable_v<Fn, double> ||
            std::is_invocable_v<Fn, std::string> || std::is_invocable_v<Fn>,
        "[patternia.pred]: The provided object is not callable. "
        "Ensure pred(...) receives a function, lambda, or callable object.");

    // Check if it returns bool when called with int (common case)
    if constexpr (std::is_invocable_v<Fn, int>) {
      using R = std::invoke_result_t<Fn, int>;
      static_assert(
          std::is_convertible_v<R, bool>,
          "[patternia.pred]: Predicate must return a type convertible to bool. "
          "For example: pred([](auto v){ return v > 0; })");
    }

    return detail::predicate_pattern<Fn>(std::forward<F>(f));
  }

} // namespace ptn::pattern::value
