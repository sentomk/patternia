#pragma once

#include <utility>

#include "ptn/pattern/value/detail/literal_pattern.hpp"
#include "ptn/config.hpp"

/**
 * @file literal.hpp
 * @brief Public API for literal value patterns (`lit()` and `lit_ci()`).
 */

namespace ptn::pattern::value {

  /**
   * @brief Factory for literal_pattern.
   *
   * Ensures:
   *   - literal value type is valid for storage and comparison
   */
  template <typename V>
  constexpr auto lit(V &&v) {
    using store_t = detail::literal_store_t<V>; // custom store type

    // store_t must not be void
    static_assert(
        !std::is_void_v<store_t>,
        "[patternia.lit]: Literal value cannot be of type void.");

    // store_t must not be a reference
    static_assert(
        !std::is_reference_v<store_t>,
        "[patternia.lit]: Literal value must be a value type (non-reference).");

    // store_t must be move-constructible
    static_assert(
        std::is_move_constructible_v<store_t>,
        "[patternia.lit]: Literal value must be move-constructible.");

    // store_t must support equality comparison
    static_assert(
        std::is_constructible_v<
            bool,
            decltype(std::declval<const store_t &>() == std::declval<const store_t &>())>,
        "[patternia.lit]: Literal value type must support operator==.");

    return detail::literal_pattern<store_t>(store_t(std::forward<V>(v)));
  }

  /**
   * @brief Case-insensitive value-pattern factory.
   */
  template <typename V>
  constexpr auto lit_ci(V &&v) {
    using store_t = detail::literal_store_t<V>;
    return detail::literal_pattern<store_t, detail::iequal_ascii>(
        store_t(std::forward<V>(v)), detail::iequal_ascii{});
  }

} // namespace ptn::pattern::value
