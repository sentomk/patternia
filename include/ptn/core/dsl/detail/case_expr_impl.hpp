#pragma once

#include <tuple>

// Internal implementation of case expressions.
//
// This header contains the internal case expression structure that represents
// pattern-handler pairs created by the >> operator. This is an implementation
// detail and should not be used directly.

namespace ptn::core::dsl::detail {

  /// Internal representation of a pattern-handler pair (case)
  ///
  /// This struct is created implicitly when using the >> operator:
  /// ```cpp
  /// pattern >> handler  // Creates a case_expr internally
  /// ```
  ///
  /// @tparam Pattern The pattern type to match against
  /// @tparam Handler The handler type to execute on match
  ///
  /// @internal This is an internal implementation detail. Use the >> operator.
  template <typename Pattern, typename Handler>
  struct case_expr {
    using pattern_type = Pattern;
    using handler_type = Handler;

    Pattern pattern; ///< The pattern to match against
    Handler handler; ///< The handler to execute if pattern matches
  };

  template <typename... Cases>
  struct cases_pack {
    using tuple_type = std::tuple<Cases...>;
    tuple_type cases;

    constexpr explicit cases_pack(tuple_type t) : cases(std::move(t)) {
    }
  };

} // namespace ptn::core::dsl::detail
