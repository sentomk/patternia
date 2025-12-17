#pragma once

// Entry function for Core matching engine.
//
// This header provides the main entry points for Patternia's pattern matching
// functionality, supporting both type-deduced and explicit-type matching.

#include <type_traits>

#include "ptn/core/engine/detail/builder_impl.hpp"

namespace ptn {

  // Primary entry for pattern matching.
  // Subject type is automatically deduced as std::decay_t<T>.
  // Usage: match(value).when(case1).when(case2).otherwise(handler)
  template <typename T>
  constexpr auto match(T &&value) {
    using V = std::decay_t<T>;

    // Initial builder state:
    //   - no cases
    //   - no match-level fallback
    return core::engine::detail::match_builder<
        V,
        false /* HasMatchFallback */
        >::create(V(std::forward<T>(value)));
  }

  // Explicit-typed entry for pattern matching: match<U>(value)
  // This overload is SFINAE-disabled if U and T are the same after decay,
  // to avoid ambiguous calls with the primary overload.
  // Usage: match<TargetType>(value).when(case).otherwise(handler)
  template <typename U, typename T>
  constexpr auto match(T &&value) -> std::enable_if_t<
      !std::is_same_v<std::decay_t<U>, std::decay_t<T>>,
      core::engine::detail::match_builder<
          std::decay_t<U>,
          false /* HasMatchFallback */
          >> {

    using Subject  = std::decay_t<U>;
    using ValueRaw = std::decay_t<T>;

    constexpr bool subject_constructible =
        std::is_constructible_v<Subject, ValueRaw> ||
        // Specialized: integral → enum is allowed
        (std::is_enum_v<Subject> && std::is_integral_v<ValueRaw>);

    static_assert(
        subject_constructible,
        "[ptn::match<U>(value)]: explicit subject type U cannot be "
        "constructed from the given value.");

    Subject subject = static_cast<Subject>(std::forward<T>(value));

    return core::engine::detail::match_builder<
        Subject,
        false /* HasMatchFallback */
        >::create(std::move(subject));
  }

} // namespace ptn
