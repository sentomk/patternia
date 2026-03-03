#pragma once

// Entry function for Core matching engine.
//
// This header provides the main entry points for Patternia's pattern matching
// functionality, supporting both type-deduced and explicit-type matching.

#include <type_traits>
#include <utility>

#include "ptn/core/engine/detail/builder_impl.hpp"

namespace ptn {

  // Primary entry for pattern matching.
  // Subject type is automatically deduced as std::decay_t<T>.
  // Usage:
  //   - match(value).when(case1).when(case2).otherwise(handler)
  //   - match(value) | on(case1, case2, ..., __ >> fallback)
  template <typename T>
  constexpr auto match(T &value) {
    using V = T &;

    // Initial builder state:
    //   - no cases
    //   - no match-level fallback
    return core::engine::detail::match_builder<
        V,
        false /* HasMatchFallback */
        >::create(V(value));
  }

  // Caches an `on(...)` matcher behind a stateless factory.
  //
  // This is an explicit wrapper around the common handwritten pattern:
  // `static auto cases = on(...);`.
  //
  // Usage:
  //   - match(value) | static_on([] { return on(..., __ >> fallback); })
  //
  // The factory must be stateless so caching does not silently freeze runtime
  // captures on the first call.
  template <typename Factory>
  inline decltype(auto) static_on(Factory &&factory) {
    using factory_t = std::decay_t<Factory>;
    static_assert(
        std::is_empty_v<factory_t>,
        "[Patternia.static_on]: factory must be stateless. "
        "Tip: pass [] { return on(...); }.");
    static_assert(
        std::is_invocable_v<factory_t &>,
        "[Patternia.static_on]: factory must be invocable with no "
        "arguments.");

    using cases_t = std::decay_t<std::invoke_result_t<factory_t &>>;
    static_assert(
        core::dsl::detail::is_on_v<cases_t>,
        "[Patternia.static_on]: factory must return on(...).");

    static cases_t cases = std::forward<Factory>(factory)();
    return (cases);
  }

} // namespace ptn
