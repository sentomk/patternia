#pragma once

#include <tuple>
#include <type_traits>
#include <utility>

// Internal implementation of case expressions.
//
// This header contains the internal case expression structure that represents
// pattern-handler pairs created by the >> operator. This is an implementation
// detail and should not be used directly.

namespace ptn::core::dsl::detail {

  // Internal representation of a pattern-handler pair (case).
  //
  // This struct is created implicitly when using the >> operator:
  // ```cpp
  // pattern >> handler  // Creates a case_expr internally.
  // ```
  //
  // Pattern is the pattern type to match against.
  // Handler is the handler type to execute on match.
  //
  // Internal implementation detail. Prefer using the >> operator.
  template <typename Pattern, typename Handler>
  struct case_expr {
    using pattern_type = Pattern;
    using handler_type = Handler;

    Pattern pattern; // The pattern to match against.
    Handler handler; // The handler to execute if the pattern matches.
  };

  // Compact case pack used by `match(x) | on{ ... }`.
  template <typename... Cases>
  struct on {
    using tuple_type = std::tuple<Cases...>;

    tuple_type cases;

    constexpr explicit on(Cases... exprs)
        : cases(std::forward<Cases>(exprs)...) {
    }
  };

  template <typename... Cases>
  on(Cases...) -> on<std::decay_t<Cases>...>;

} // namespace ptn::core::dsl::detail
