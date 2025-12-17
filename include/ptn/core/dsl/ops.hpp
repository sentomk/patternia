#pragma once

// DSL operators for building case expressions and composing patterns.
//
// This header provides operator overloads that enable Patternia's intuitive
// domain-specific language for pattern matching, particularly the >> operator
// for creating case expressions.

#include <type_traits>
#include <utility>

#include "ptn/core/common/common_traits.hpp"
#include "ptn/core/dsl/detail/case_expr_impl.hpp"

namespace ptn::core::dsl::ops {

  // Operator Overloads

  /// Pattern >> Handler / Pattern >> Value operator
  ///
  /// Creates case expressions by connecting patterns with handlers or values.
  /// Supports two distinct modes:
  ///
  /// 1. Value Mode: If Handler is a value type (int, enum, string, etc.),
  ///    constructs a value-handler that ignores bound arguments and always
  ///    returns that value. Enables syntax like: `pattern >> 42`
  ///
  /// 2. Handler Mode: If Handler is invocable (function, lambda, etc.),
  ///    treats it as a proper handler that receives bound arguments.
  ///    Enables syntax like: `pattern >> [](auto x) { return x * 2; }`
  ///
  /// This distinction prevents parameterized lambdas from being misidentified
  /// as values, avoiding closure type issues in std::common_type_t resolution.
  template <typename Pattern, typename Handler>
  constexpr auto operator>>(Pattern &&pattern, Handler &&handler) {
    using P = std::decay_t<Pattern>;
    using H = std::decay_t<Handler>;

    if constexpr (ptn::core::traits::detail::is_value_like_v<H>) {
      // Value Mode: Pattern >> Value syntax sugar
      auto value_handler = [val = std::forward<Handler>(handler)](
                               auto &&...) -> H { return val; };

      return core::dsl::detail::case_expr<P, decltype(value_handler)>{
          std::forward<Pattern>(pattern), std::move(value_handler)};
    }
    else {
      // Handler Mode: Pattern >> Handler
      return core::dsl::detail::case_expr<P, H>{
          std::forward<Pattern>(pattern), std::forward<Handler>(handler)};
    }
  }
} // namespace ptn::core::dsl::ops
