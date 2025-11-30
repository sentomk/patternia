#pragma once
/**
 * @file type_of.hpp
 * @brief Provides `type_of(obj)`, converting a runtime object into a
 *        `type_tag<T>` for type-based pattern matching.
 *
 * This function is the gateway to the Type Layer.
 *
 * Part of Pattern Layer (namespace ptn::pattern::type)
 */

#include <type_traits>
#include "ptn/pattern/type.hpp"

namespace ptn::pat::type {

  /**
   * @brief Returns a `type_tag` representing the static type of `obj`.
   *
   * This allows type-level matching.
   */
  template <typename T>
  constexpr auto type_of(const T &) noexcept {
    return type_tag<std::decay_t<T>>{};
  }

} // namespace ptn::pat::type