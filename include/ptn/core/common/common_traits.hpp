#pragma once

/**
 * @file common_traits.hpp
 * @brief Core type traits used by the matching engine.
 */

#include "ptn/config.hpp"
#include "ptn/pattern/base/fwd.h"
#include <type_traits>
#include <tuple>
#include <utility>
#include <string>
#include <string_view>

#if PTN_USE_CONCEPTS
#include <concepts>
#endif

// Forward declaration
namespace ptn::core::dsl::detail {
  template <typename Pattern, typename Handler>
  struct case_expr;
}

namespace ptn::core::common {

  /** Basic Type Extraction */

  /// @brief Detects if a type is a `case_expr`.
  template <typename T>
  struct is_case_expr : std::false_type {};

  template <typename Pattern, typename Handler>
  struct is_case_expr<dsl::detail::case_expr<Pattern, Handler>>
      : std::true_type {};

  template <typename T>
  inline constexpr bool is_case_expr_v = is_case_expr<T>::value;

  /// @brief Extracts the `Pattern` type from a `case_expr`.
  template <typename Case>
  struct case_pattern {
    using type = void;
  };

  template <typename Pattern, typename Handler>
  struct case_pattern<dsl::detail::case_expr<Pattern, Handler>> {
    using type = Pattern;
  };

  template <typename Case>
  using case_pattern_t = typename case_pattern<Case>::type;

  /// @brief Extracts the `Handler` type from a `case_expr`.
  template <typename Case>
  struct case_handler {
    using type = void;
  };

  template <typename Pattern, typename Handler>
  struct case_handler<dsl::detail::case_expr<Pattern, Handler>> {
    using type = Handler;
  };

  template <typename Case>
  using case_handler_t = typename case_handler<Case>::type;

  /** Handler Invocability Check */
#if PTN_USE_CONCEPTS
  /// @brief C++20 Concept to check if a handler can be invoked with arguments
  /// bound by its pattern.
  template <typename Case, typename Subject>
  concept handler_invocable_for = requires(Case &&c, Subject &&s) {
    // Simulates the call: pattern.bind(s) -> args..., handler(args...)
    std::apply(
        std::forward<Case>(c).handler,
        std::tuple_cat(
            std::forward_as_tuple(std::forward<Subject>(s)),
            std::forward<Case>(c).pattern.bind(std::forward<Subject>(s))));
  };

  template <typename Case, typename Subject>
  inline constexpr bool is_handler_invocable_v =
      handler_invocable_for<Case, Subject>;
#else
  /// @brief C++17 SFINAE implementation for handler invocability.
  namespace detail {
    template <typename H, typename Tuple>
    static constexpr std::true_type is_applicable_impl(
        decltype(std::apply(std::declval<H>(), std::declval<Tuple>())) *);

    template <typename H, typename Tuple>
    static constexpr std::false_type is_applicable_impl(...);

    // Detects "value-like" types to enable `pattern >> value` syntax sugar.
    template <typename T>
    struct is_value_like_impl {
      using D = std::decay_t<T>;

      static constexpr bool value = std::is_arithmetic_v<D> ||
                                    std::is_enum_v<D> ||
                                    std::is_same_v<D, std::string> ||
                                    std::is_same_v<D, std::string_view> ||
                                    std::is_convertible_v<D, std::string_view>;
    };

    template <typename T>
    inline constexpr bool is_value_like_v =
        is_value_like_impl<std::decay_t<T>>::value;
  } // namespace detail

  template <typename Case, typename Subject>
  struct is_handler_invocable {
  private:
    using handler_type     = case_handler_t<Case>;
    using pattern_type     = case_pattern_t<Case>;
    using bound_args_tuple = pat::base::binding_args_t<pattern_type, Subject>;

    using full_invoke_args_tuple = decltype(std::tuple_cat(
        std::declval<std::tuple<const Subject &>>(),
        std::declval<bound_args_tuple>()));

  public:
    static constexpr bool value =
        decltype(detail::
                     is_applicable_impl<handler_type, full_invoke_args_tuple>(
                         nullptr))::value;
  };

  template <typename Case, typename Subject>
  inline constexpr bool is_handler_invocable_v =
      is_handler_invocable<Case, Subject>::value;
#endif

  /** Case and Match Result Types */

  /// @brief Computes the result type of a single case expression.
  template <typename Subject, typename Case>
  struct case_result {
  private:
    using handler_type     = case_handler_t<Case>;
    using pattern_type     = case_pattern_t<Case>;
    using bound_args_tuple = pat::base::binding_args_t<pattern_type, Subject>;

    using full_invoke_args_tuple = decltype(std::tuple_cat(
        std::declval<std::tuple<const Subject &>>(),
        std::declval<bound_args_tuple>()));

  public:
    using type = decltype(std::apply(
        std::declval<handler_type>(), std::declval<full_invoke_args_tuple>()));
  };

  template <typename Subject, typename Case>
  using case_result_t = typename case_result<Subject, Case>::type;

  namespace detail {

    /// @brief Helper to deduce the result type of an `otherwise` handler.
    template <typename O, typename S>
    static constexpr auto get_otherwise_result_impl(int)
        -> decltype(std::declval<O>()(std::declval<S &>()));

    template <typename O, typename S>
    static constexpr auto get_otherwise_result_impl(...)
        -> decltype(std::declval<O>()());

  } // namespace detail

  template <typename Otherwise, typename Subject>
  using otherwise_result_t =
      decltype(detail::get_otherwise_result_impl<Otherwise, Subject>(0));

  /// @brief Computes the common result type of the entire match expression.
  template <typename Subject, typename Otherwise, typename... Cases>
  struct match_result {
    using type = std::common_type_t<
        std::invoke_result_t<typename Cases::handler_type, Subject &>...,
        std::invoke_result_t<Otherwise, Subject &>>;
  };

  template <typename Subject, typename Otherwise, typename... Cases>
  using match_result_t =
      typename match_result<Subject, Otherwise, Cases...>::type;

} // namespace ptn::core::common
