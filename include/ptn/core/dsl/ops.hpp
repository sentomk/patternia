#pragma once

// DSL operators for building case expressions and composing patterns.

#include <type_traits>
#include <utility>

#include "ptn/core/common/common_traits.hpp"
#include "ptn/core/dsl/detail/case_expr_impl.hpp"

namespace ptn::core::dsl::ops {

  // Operator Overloads

  // `operator>>`: Pattern >> Handler / Pattern >> Value
  //
  // This operator supports two modes:
  // 1. If Handler is a "value type" (int, enum, string, etc.), it constructs
  //    a value-handler that ignores all arguments and always returns that
  //    value.
  // 2. Otherwise, it is treated as a proper handler that can be invoked
  //    according to the rules in case_result_t.
  //
  // This prevents lambdas with parameters from being misidentified as values,
  // avoiding closure types being passed into std::common_type_t and resolving
  // common_type errors.
  template <typename Pattern, typename Handler>
  constexpr auto operator>>(Pattern &&pattern, Handler &&handler) {
    using P = std::decay_t<Pattern>;
    using H = std::decay_t<Handler>;

    if constexpr (ptn::core::common::detail::is_value_like_v<H>) {
      // Pattern >> Value syntax sugar
      auto value_handler = [val = std::forward<Handler>(handler)](
                               auto &&...) -> H { return val; };

      return core::dsl::detail::case_expr<P, decltype(value_handler)>{
          std::forward<Pattern>(pattern), std::move(value_handler)};
    }
    else {
      // Pattern >> Handler
      return core::dsl::detail::case_expr<P, H>{
          std::forward<Pattern>(pattern), std::forward<Handler>(handler)};
    }
  }
} // namespace ptn::core::dsl::ops
