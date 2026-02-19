#pragma once

#include <tuple>

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

  template <typename... Cases>
  struct cases_pack {
    using tuple_type = std::tuple<Cases...>;
    tuple_type cases;

    constexpr explicit cases_pack(tuple_type t) : cases(std::move(t)) {
    }
  };

} // namespace ptn::core::dsl::detail
