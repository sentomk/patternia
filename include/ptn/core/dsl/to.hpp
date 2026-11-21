#pragma once

// `to<T>`: a callable that constructs a `T` from its arguments.
//
// It is an ordinary handler, so it runs only when its case is
// selected:
//
//   $(has<&P::x, &P::y>) >> to<Point>     // Point{x, y}
//   $ >> _ * 2 | to<Celsius>               // Celsius{value * 2}
//
// An aggregate is initialized with braces, `T{args...}`; any other
// type with parentheses, `T(args...)`, so that a type with an
// initializer-list constructor such as `std::vector<int>` is not
// given one by accident.

#include <type_traits>
#include <utility>

#include "ptn/core/common/inline.hpp"

namespace ptn::core::dsl {

  namespace detail {

    template <typename... Args>
    struct to_args {};

    template <typename T, typename Args, typename = void>
    struct is_brace_constructible : std::false_type {};

    template <typename T, typename... Args>
    struct is_brace_constructible<
        T,
        to_args<Args...>,
        std::void_t<decltype(T{std::declval<Args>()...})>>
        : std::true_type {};

    template <typename T, typename... Args>
    inline constexpr bool is_to_constructible_v = std::conditional_t<
        std::is_aggregate_v<T>,
        is_brace_constructible<T, to_args<Args...>>,
        std::is_constructible<T, Args...>>::value;

  } // namespace detail

  namespace detail {
    template <typename T>
    inline constexpr bool is_to_target_v = std::is_object_v<T>
                                           && !std::is_array_v<T>
                                           && !std::is_abstract_v<T>;

    // `int` stands in for an invalid T, so that only to_fn's own
    // diagnostic is reported, not a function returning an array.
    template <typename T>
    using to_result_t = std::
        conditional_t<is_to_target_v<T>, T, int>;
  } // namespace detail

  template <typename T>
  struct to_fn {
    static_assert(detail::is_to_target_v<T>,
                  "[Patternia.to]: to<T> needs T to be a complete, "
                  "non-array, non-abstract object type.");
    static_assert(std::is_same_v<T, std::remove_cv_t<T>>,
                  "[Patternia.to]: to<T> returns a new value; drop "
                  "const or volatile from T.");

    template <typename... Args,
              std::enable_if_t<
                  !detail::is_to_target_v<T>
                      || detail::is_to_constructible_v<T, Args...>,
                  int> = 0>
    PTN_DETAIL_ALWAYS_INLINE constexpr detail::to_result_t<T>
    operator()(Args &&...args) const {
      if constexpr (!detail::is_to_target_v<T>) {
        return 0;
      }
      else if constexpr (std::is_aggregate_v<T>) {
        return T{std::forward<Args>(args)...};
      }
      else {
        return T(std::forward<Args>(args)...);
      }
    }
  };

  // Constructs a `T` from the arguments it is called with.
  template <typename T>
  inline constexpr to_fn<T> to{};

} // namespace ptn::core::dsl
