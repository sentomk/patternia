#pragma once

// DSL operators for building case expressions and composing
// patterns.
//
// This header provides operator overloads that enable Patternia's
// intuitive domain-specific language for pattern matching,
// particularly the >> operator for creating case expressions.

#include <string_view>
#include <type_traits>
#include <utility>

#include "ptn/core/common/common_traits.hpp"
#include "ptn/core/dsl/detail/case_expr_impl.hpp"
#include "ptn/pattern/lit.hpp"

namespace ptn::pat::base {

  // Operator overloads.

  // Pattern >> Handler / Pattern >> Value operator.
  //
  // Creates case expressions by connecting patterns with handlers or
  // values. Supports two distinct modes:
  //
  // 1. Value mode: If Handler is a value type (int, enum, string,
  // etc.),
  //    constructs a value-handler that ignores bound arguments and
  //    always returns that value. Enables syntax like `pattern >>
  //    42`.
  //
  // 2. Handler mode: If Handler is invocable (function, lambda,
  // etc.),
  //    treats it as a proper handler that receives bound arguments.
  //    Enables syntax like `pattern >> [](auto x) { return x * 2;
  //    }`.
  //
  // This distinction prevents parameterized lambdas from being
  // misidentified as values, avoiding closure type issues in
  // std::common_type_t resolution. The operator lives in
  // `ptn::pat::base` so ADL can find it for built-in patterns even
  // when user code stays fully qualified and does not add `using
  // namespace ptn;`.
  template <
      typename Pattern,
      typename Handler,
      std::enable_if_t<std::is_base_of_v<ptn::pat::base::pattern_tag,
                                         std::decay_t<Pattern>>,
                       int> = 0>
  constexpr auto operator>>(Pattern &&pattern, Handler &&handler) {
    using P = std::decay_t<Pattern>;
    using H = std::decay_t<Handler>;

    if constexpr (ptn::core::traits::detail::is_value_like_v<H>) {
      // Value Mode: Pattern >> Value syntax sugar
      using value_handler_t = core::dsl::detail::value_handler<H>;

      return core::dsl::detail::case_expr<P, value_handler_t>{
          std::forward<Pattern>(pattern),
          value_handler_t{std::forward<Handler>(handler)}};
    }
    else {
      // Handler Mode: Pattern >> Handler
      return core::dsl::detail::case_expr<P, H>{
          std::forward<Pattern>(pattern),
          std::forward<Handler>(handler)};
    }
  }

  // Implicit literal wrapping: Value >> Handler.
  //
  // When the left operand is not a pattern but is a value-like type
  // (arithmetic, enum, string literal, etc.), it is automatically
  // wrapped in lit() to form a literal pattern. This enables concise
  // syntax like:
  //   42 >> handler
  //   "hello" >> handler
  //
  // The constraint excludes callables (lambdas, function pointers)
  // to prevent accidental capture.
  template <typename Value,
            typename Handler,
            std::enable_if_t<
                !std::is_base_of_v<ptn::pat::base::pattern_tag,
                                   std::decay_t<Value>>
                    && (std::is_arithmetic_v<std::decay_t<Value>>
                        || std::is_enum_v<std::decay_t<Value>>
                        || std::is_convertible_v<std::decay_t<Value>,
                                                 std::string_view>),
                int> = 0>
  constexpr auto operator>>(Value &&value, Handler &&handler) {
    return ptn::pat::lit(std::forward<Value>(value))
           >> std::forward<Handler>(handler);
  }

} // namespace ptn::pat::base

namespace ptn::core::dsl::ops {

  using ptn::pat::base::operator>>;

} // namespace ptn::core::dsl::ops
