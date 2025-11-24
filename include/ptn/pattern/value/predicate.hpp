#pragma once

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
    return detail::predicate_pattern<Fn>(std::forward<F>(f));
  }

} // namespace ptn::pattern::value
