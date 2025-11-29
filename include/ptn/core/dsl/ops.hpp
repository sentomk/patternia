#pragma once

/**
 * @file ops.hpp
 * @brief DSL operators for building case expressions and composing patterns.
 */

#include <type_traits>
#include <utility>
#include <string>
#include <string_view>

#include "ptn/pattern/value/predicate.hpp"
#include "ptn/core/dsl/detail/case_expr_impl.hpp"
#include "ptn/pattern/base/pattern_traits.hpp"

namespace ptn::core::dsl::ops {

  // --- Internal Implementation Details ---

  namespace detail {

    /**
     * @brief Detects "value-like" types to enable `pattern >> value` syntax
     * sugar.
     *
     * Only types that are clearly values (arithmetic, enum, string) are
     * considered value-like. Other types (lambdas, function objects, function
     * pointers) are treated as handlers.
     */
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

  // --- Operator Overloads ---

  /**
   * @brief `operator>>`: Pattern >> Handler / Pattern >> Value
   *
   * This operator supports two modes:
   * 1. If Handler is a "value type" (int, enum, string, etc.), it constructs
   *    a value-handler that ignores all arguments and always returns that
   *    value.
   * 2. Otherwise, it is treated as a proper handler that can be invoked
   *    according to the rules in case_result_t.
   *
   * This prevents lambdas with parameters from being misidentified as values,
   * avoiding closure types being passed into std::common_type_t and resolving
   * common_type errors.
   */
  template <typename Pattern, typename Handler>
  constexpr auto operator>>(Pattern &&pattern, Handler &&handler) {
    using P = std::decay_t<Pattern>;
    using H = std::decay_t<Handler>;

    // The Pattern should ideally be a valid pattern, but we do not enforce
    // this constraint here. Let case_expr_impl and pattern_traits handle more
    // detailed validation. For stricter enforcement, you can add:
    //
    // static_assert(pat::detail::is_pattern_v<P>,
    //   "[patternia]: left operand of >> must be a pattern.");

    if constexpr (detail::is_value_like_v<H>) {
      // Pattern >> Value syntax sugar
      auto value_handler = [val = std::forward<Handler>(handler)](
                               auto &&...) -> H { return val; };

      return core::dsl::detail::case_expr<P, decltype(value_handler)>{
          std::forward<Pattern>(pattern), std::move(value_handler)};
    }
    else {
      // Pattern >> Handler
      return core::dsl::detail::case_expr<P, H>{
          std::forward<Pattern>(pattern), std::forward<Handler>(handler)};
    }
  }

  /**
   * @brief Pattern logical composition: &&, ||, !
   *
   * These operators enable logical composition of patterns using
   * `pat::detail::is_pattern_v<T>` from pattern/base/pattern_traits.hpp.
   * They are only enabled for actual pattern types.
   */

  /// @brief AND composition: p1 && p2
  template <
      typename L,
      typename R,
      typename = std::enable_if_t<
          pat::detail::is_pattern_v<std::decay_t<L>> &&
          pat::detail::is_pattern_v<std::decay_t<R>>>>
  constexpr auto operator&&(L &&l, R &&r) {
    return ptn::pat::value::detail::
        and_pattern<std::decay_t<L>, std::decay_t<R>>(
            std::forward<L>(l), std::forward<R>(r));
  }

  /// @brief OR composition: p1 || p2
  template <
      typename L,
      typename R,
      typename = std::enable_if_t<
          pat::detail::is_pattern_v<std::decay_t<L>> &&
          pat::detail::is_pattern_v<std::decay_t<R>>>>
  constexpr auto operator||(L &&l, R &&r) {
    return ptn::pat::value::detail::
        or_pattern<std::decay_t<L>, std::decay_t<R>>(
            std::forward<L>(l), std::forward<R>(r));
  }

  /// @brief NOT composition: !p
  template <
      typename P,
      typename = std::enable_if_t<pat::detail::is_pattern_v<std::decay_t<P>>>>
  constexpr auto operator!(P &&p) {
    return ptn::pat::value::detail::not_pattern<std::decay_t<P>>(
        std::forward<P>(p));
  }

} // namespace ptn::core::dsl::ops
