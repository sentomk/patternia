#pragma once

// Entry function for Core matching engine.
//
// This header provides the main entry points for Patternia's pattern matching
// functionality, supporting both type-deduced and explicit-type matching.

#include "ptn/core/engine/detail/builder_impl.hpp"

namespace ptn {

  // Primary entry for pattern matching.
  // Subject type is automatically deduced as std::decay_t<T>.
  // Usage:
  //   - match(value).when(case1).when(case2).otherwise(handler)
  //   - match(value) | on{case1, case2, ..., __ >> fallback}
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

} // namespace ptn
