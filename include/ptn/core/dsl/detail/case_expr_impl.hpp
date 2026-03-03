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

  template <typename... Cases>
  struct on;

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

#if defined(__cpp_no_unique_address) && __cpp_no_unique_address >= 201803L
    [[no_unique_address]] Pattern pattern; // The pattern to match against.
    [[no_unique_address]] Handler handler; // The handler to execute if matched.
#else
    Pattern pattern; // The pattern to match against.
    Handler handler; // The handler to execute if matched.
#endif
  };

  template <typename T>
  struct value_handler {
    T value;

    template <typename... Args>
    constexpr T operator()(Args &&...) const {
      return value;
    }
  };

  template <typename Handler>
  struct is_value_handler : std::false_type {};

  template <typename T>
  struct is_value_handler<value_handler<T>> : std::true_type {};

  template <typename Handler>
  inline constexpr bool
      is_value_handler_v = is_value_handler<std::remove_cv_t<
          std::remove_reference_t<Handler>>>::value;

  template <typename T>
  struct is_on : std::false_type {};

  template <typename... Cases>
  struct is_on<on<Cases...>> : std::true_type {};

  template <typename T>
  inline constexpr bool
      is_on_v = is_on<std::remove_cv_t<std::remove_reference_t<T>>>::value;

  // Compact case pack used by `match(x) | on(...)`.
  template <typename... Cases>
  struct on {
    using tuple_type = std::tuple<Cases...>;

    tuple_type cases;

    template <typename... Exprs,
              typename = std::enable_if_t<sizeof...(Exprs) == sizeof...(Cases)>>
    constexpr explicit on(Exprs &&...exprs)
        : cases(std::forward<Exprs>(exprs)...) {
    }
  };

  template <typename... Cases>
  on(Cases &&...) -> on<std::decay_t<Cases>...>;

} // namespace ptn::core::dsl::detail
