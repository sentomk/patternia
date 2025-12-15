#pragma once

// Core type traits used by the matching engine.
//
// This header provides fundamental type traits and utilities for pattern
// matching, including case expression detection, handler invocability checks,
// and result type deduction.

#include "ptn/pattern/base/fwd.h"
#include <cstddef>
#include <type_traits>
#include <tuple>
#include <utility>

// Forward declaration
namespace ptn::core::dsl::detail {
  template <typename Pattern, typename Handler>
  struct case_expr;
}

namespace ptn::core::common {

  // Basic Type Extraction

  // Detects if a type is a `case_expr`.
  template <typename T>
  struct is_case_expr : std::false_type {};

  template <typename Pattern, typename Handler>
  struct is_case_expr<dsl::detail::case_expr<Pattern, Handler>>
      : std::true_type {};

  template <typename T>
  inline constexpr bool is_case_expr_v = is_case_expr<T>::value;

  // Extracts the `Pattern` type from a `case_expr`.
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

  // Extracts the `Handler` type from a `case_expr`.
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

  // Handler Invocability Check (C++17)

  namespace detail {
    template <typename H, typename Tuple>
    static constexpr std::true_type is_applicable_impl(
        decltype(std::apply(std::declval<H>(), std::declval<Tuple>())) *);

    template <typename H, typename Tuple>
    static constexpr std::false_type is_applicable_impl(...);

    // Detect if a type is "handler-like".
    template <typename T>
    struct is_handler_like_impl {
    private:
      using D = std::decay_t<T>;

      template <typename U>
      static auto test_op(int) -> decltype(&U::operator(), std::true_type{});

      // Generic lambdas have a templated operator(). Taking the address of
      // `U::operator()` is ill-formed in that case, so we probe a few common
      // template arities explicitly. This allows us to correctly classify
      // generic lambdas (e.g. [](auto x){...}) as handler-like.
      template <typename U>
      static auto test_op_t1(int)
          -> decltype(&U::template operator()<int>, std::true_type{});

      template <typename U>
      static auto test_op_t2(int)
          -> decltype(&U::template operator()<int, int>, std::true_type{});

      template <typename U>
      static auto test_op_t3(int)
          -> decltype(&U::template operator()<int, int, int>, std::true_type{});

      template <typename U>
      static auto test_op_t4(int)
          -> decltype(&U::template operator()<int, int, int, int>, std::true_type{});

      template <typename>
      static auto test_op(...) -> std::false_type;

      template <typename>
      static auto test_op_t1(...) -> std::false_type;

      template <typename>
      static auto test_op_t2(...) -> std::false_type;

      template <typename>
      static auto test_op_t3(...) -> std::false_type;

      template <typename>
      static auto test_op_t4(...) -> std::false_type;

      static constexpr bool has_call_operator =
          decltype(test_op<D>(0))::value || decltype(test_op_t1<D>(0))::value ||
          decltype(test_op_t2<D>(0))::value ||
          decltype(test_op_t3<D>(0))::value ||
          decltype(test_op_t4<D>(0))::value;

      static constexpr bool is_function_type = std::is_function_v<D>;

      static constexpr bool is_function_pointer =
          std::is_pointer_v<D> && std::is_function_v<std::remove_pointer_t<D>>;

      static constexpr bool is_function_reference =
          std::is_reference_v<T> &&
          std::is_function_v<std::remove_reference_t<T>>;

    public:
      static constexpr bool value = has_call_operator || is_function_type ||
                                    is_function_pointer ||
                                    is_function_reference;
    };

    template <typename T>
    inline constexpr bool is_handler_like_v = is_handler_like_impl<T>::value;

    // "Value-like" is everything that is not "handler-like".
    template <typename T>
    inline constexpr bool is_value_like_v = !is_handler_like_v<T>;
  } // namespace detail

  // Checks whether a case's handler is invocable for a given subject type.
  //
  // NOTE (Design A):
  //   Handler arguments are exactly the pattern's bound values:
  //   handler( pattern.bind(subject)... )
  //   The subject itself is NOT passed as a separate first argument.
  template <typename Case, typename Subject>
  struct is_handler_invocable {
  private:
    using handler_type     = case_handler_t<Case>;
    using pattern_type     = case_pattern_t<Case>;
    using bound_args_tuple = pat::base::binding_args_t<pattern_type, Subject>;

    // Under Design A, the full argument tuple is exactly the bound args tuple.
    using full_invoke_args_tuple = bound_args_tuple;

  public:
    static constexpr bool value =
        decltype(detail::
                     is_applicable_impl<handler_type, full_invoke_args_tuple>(
                         nullptr))::value ||
        std::is_invocable_v<handler_type>;
  };

  template <typename Case, typename Subject>
  inline constexpr bool is_handler_invocable_v =
      is_handler_invocable<Case, Subject>::value;

  // Case and Match Result Types

  // Computes the result type of a single case expression.
  template <typename Subject, typename Case>
  struct case_result {
  private:
    // Extract handler and pattern types from the case expression.
    using handler_type = case_handler_t<Case>;
    using pattern_type = case_pattern_t<Case>;

    // Get the tuple of bound arguments from pattern matching.
    using bound_args_tuple = pat::base::binding_args_t<pattern_type, Subject>;

    // Under Design A, handler is invoked with exactly the bound arguments.
    using full_invoke_args_tuple = bound_args_tuple;

  public:
    // Helper to deduce the result type of a case handler.
    //
    // Design note:
    //   - If the handler is invocable with the bound arguments, use that.
    //   - Otherwise, if it is invocable with no arguments, treat it as
    //     intentionally ignoring bound values.
    //   - Otherwise, let diagnostics/static-asserts report the mismatch.
    template <typename H, typename Tuple, std::size_t... Is>
    static constexpr auto
        invoke_result_with_tuple_impl(std::index_sequence<Is...>)
            -> std::invoke_result_t<H, std::tuple_element_t<Is, Tuple>...>;

    template <typename H, typename Tuple>
    using invoke_result_with_tuple_t =
        decltype(invoke_result_with_tuple_impl<H, Tuple>(
            std::make_index_sequence<std::tuple_size_v<Tuple>>{}));

    template <typename H, typename Tuple>
    static constexpr auto get_case_result_impl(int)
        -> invoke_result_with_tuple_t<H, Tuple>;

    template <typename H, typename Tuple>
    static constexpr auto get_case_result_impl(...) -> std::invoke_result_t<H>;

  public:
    // The result type when the handler is invoked.
    using type =
        decltype(get_case_result_impl<handler_type, full_invoke_args_tuple>(0));
  };

  template <typename Subject, typename Case>
  using case_result_t = typename case_result<Subject, Case>::type;

  namespace detail {

    // Helper to deduce the result type of an `otherwise` handler.
    // First overload: handler takes subject as parameter.
    template <typename O, typename S>
    static constexpr auto get_otherwise_result_impl(int)
        -> decltype(std::declval<O>()(std::declval<S &>()));

    // Second overload: handler takes no parameters.
    template <typename O, typename S>
    static constexpr auto get_otherwise_result_impl(...)
        -> decltype(std::declval<O>()());

  } // namespace detail

  template <typename Otherwise, typename Subject>
  using otherwise_result_t =
      decltype(detail::get_otherwise_result_impl<Otherwise, Subject>(0));

  // Computes the common result type of the entire match expression.
  template <typename Subject, typename Otherwise, typename... Cases>
  struct match_result {

  private:
    // Result type of the otherwise handler.
    using otherwise_result = otherwise_result_t<Otherwise, Subject>;

    // Tuple containing result types of all case expressions.
    using cases_result_tuple = std::tuple<case_result_t<Subject, Cases>...>;

    // Helper function to check if all case results are void type.
    template <typename Tuple, std::size_t... Is>
    static constexpr bool all_cases_void_impl(std::index_sequence<Is...>) {
      return (std::is_void_v<std::tuple_element_t<Is, Tuple>> && ...);
    }

    // Check if all case handlers return void.
    static constexpr bool all_cases_void =
        all_cases_void_impl<cases_result_tuple>(
            std::make_index_sequence<sizeof...(Cases)>{});

    // Check if the otherwise handler returns void.
    static constexpr bool otherwise_is_void = std::is_void_v<otherwise_result>;

    // Helper to select result type without instantiating std::common_type_t
    // on void-returning (statement-style) matches.
    template <bool AllVoid, typename Dummy = void>
    struct match_result_impl;

    template <typename Dummy>
    struct match_result_impl<true, Dummy> {
      using type = void;
    };

    template <typename Dummy>
    struct match_result_impl<false, Dummy> {
      using type = std::
          common_type_t<case_result_t<Subject, Cases>..., otherwise_result>;
    };

  public:
    // Determine the common result type:
    // - If all handlers return void, the match expression returns void.
    // - Otherwise, use the common type of all handler results.
    using type =
        typename match_result_impl<all_cases_void && otherwise_is_void>::type;
  };

  template <typename Subject, typename Otherwise, typename... Cases>
  using match_result_t =
      typename match_result<Subject, Otherwise, Cases...>::type;

  // Type trait to detect if a type should be treated as void-like.
  //
  // This is used to determine if a type should be considered equivalent
  // to void for the purposes of return type deduction and validation.
  template <typename T>
  struct is_void_like : std::false_type {};

  // Specialization for void type.
  template <>
  struct is_void_like<void> : std::true_type {};

  // Specialization for function types returning void.
  template <typename... Args>
  struct is_void_like<void(Args...)> : std::true_type {};

  // Helper variable template for is_void_like trait.
  template <typename T>
  inline constexpr bool is_void_like_v = is_void_like<T>::value;

} // namespace ptn::core::common
