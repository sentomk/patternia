#pragma once

// Entry function for Core matching engine.

#include <type_traits>

#include "ptn/core/engine/detail/builder_impl.hpp"
#include "ptn/config.hpp"

namespace ptn {

  // Primary entry for pattern matching.
  //
  // Subject type is deduced as std::decay_t<T>.
  template <typename T>
  constexpr auto match(T &&value) {
    using V = std::decay_t<T>;
    // Forward into the builder with an empty case list
    return core::engine::detail::match_builder<V>::create(
        V(std::forward<T>(value)));
  }

  // Explicit-typed entry for pattern matching: match<U>(value)
  // This overload is SFINAE-disabled if U and T are the same after decay,
  // to avoid ambiguous calls with the primary overload.
  template <typename U, typename T>
  constexpr auto match(T &&value) -> std::enable_if_t<
      !std::is_same_v<std::decay_t<U>, std::decay_t<T>>,
      core::engine::detail::match_builder<U>> {
    constexpr bool subject_constructible =
        std::is_constructible_v<U, T &&> || std::is_constructible_v<U, T>;

    static_assert(
        subject_constructible,
        "[ptn::match<U>(value)]: explicit subject type U cannot be "
        "constructed from the given value.");

    U subject = static_cast<U>(std::forward<T>(value));
    return core::engine::detail::match_builder<U>::create(std::move(subject));
  }

} // namespace ptn
