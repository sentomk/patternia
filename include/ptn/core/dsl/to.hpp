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
//
// Like the lambda it stands for, `[](auto &&...args) { return
// T{args...}; }`, it accepts any arguments: a case passes it the
// pattern's bindings, and a `T` that cannot be built from them is a
// diagnostic, not a fallback to `T{}`.

#include <cstdlib>
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

    // The result is spelled out rather than deduced, so testing
    // whether to<T> is invocable never instantiates the body. It is
    // T even when the arguments cannot build one, so the match's
    // result type is unchanged and only to_fn's diagnostic is
    // reported. `int` stands in for an invalid T, so that no
    // function returns an array.
    template <typename T>
    using to_result_t = std::
        conditional_t<is_to_target_v<T>, T, int>;

    // Stands in for the T that cannot be built: the call does not
    // compile, so this never runs. A prvalue return needs no
    // constructor of T.
    template <typename T>
    [[noreturn]] T to_unbuildable() {
      std::abort();
    }
  } // namespace detail

  template <typename T>
  struct to_fn {
    static_assert(detail::is_to_target_v<T>,
                  "[Patternia.to]: to<T> needs T to be a complete, "
                  "non-array, non-abstract object type.");
    static_assert(std::is_same_v<T, std::remove_cv_t<T>>,
                  "[Patternia.to]: to<T> returns a new value; drop "
                  "const or volatile from T.");

    template <typename... Args>
    PTN_DETAIL_ALWAYS_INLINE constexpr detail::to_result_t<T>
    operator()(Args &&...args) const {
      static_assert(
          !detail::is_to_target_v<T>
              || detail::is_to_constructible_v<T, Args...>,
          "[Patternia.to]: T cannot be constructed from the "
          "arguments to<T> is called with: the case's bindings, "
          "or the result of the handler before |. to<T> is not "
          "called without them; use `_ >> to<T>` for T{}.");
      if constexpr (!detail::is_to_target_v<T>) {
        return 0;
      }
      else if constexpr (!detail::is_to_constructible_v<T,
                                                        Args...>) {
        // Keep the diagnostic above from cascading.
        return detail::to_unbuildable<T>();
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
