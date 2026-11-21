#pragma once

// Entry function for Core matching engine.
//
// This header provides the main entry points for Patternia's pattern
// matching functionality, supporting both type-deduced and
// explicit-type matching.

#include <type_traits>
#include <utility>

#include "ptn/core/common/inline.hpp"
#include "ptn/core/dsl/detail/case_expr_impl.hpp"
#include "ptn/core/dsl/detail/expression_handler.hpp"
#include "ptn/core/engine/detail/pipeline_match_context.hpp"

namespace ptn::core::engine::detail {

  template <typename Handler>
  struct is_expression_handler : std::false_type {};

  template <typename Predicate, bool ValueExpression>
  struct is_expression_handler<
      core::dsl::detail::expression_handler<Predicate,
                                            ValueExpression>>
      : std::true_type {};

  // True when `&Handler::operator()` names a single call operator
  // that is not const. Templates and overload sets (generic lambdas,
  // value handlers) are not inspected; their operators are const.
  template <typename Handler, typename = void>
  struct has_non_const_call_operator : std::false_type {};

  template <typename Member>
  struct is_non_const_member_function : std::false_type {};

  template <typename R, typename C, typename... Args>
  struct is_non_const_member_function<R (C::*)(Args...)>
      : std::true_type {};

  template <typename R, typename C, typename... Args>
  struct is_non_const_member_function<R (C::*)(Args...) noexcept>
      : std::true_type {};

  template <typename Handler>
  struct has_non_const_call_operator<
      Handler,
      std::void_t<decltype(&Handler::operator())>>
      : is_non_const_member_function<
            decltype(&Handler::operator())> {};

  // A handler that keeps no state between calls and can be called
  // through a const reference: an empty class with a const call
  // operator (such as a captureless lambda), a constant result, or
  // an expression.
  template <typename Handler>
  inline constexpr bool is_stateless_handler_v =
      (std::is_empty_v<Handler>
       && !has_non_const_call_operator<Handler>::value)
      || core::dsl::detail::is_value_handler_v<Handler>
      || is_expression_handler<Handler>::value;

  template <typename Cases>
  struct caches_cases_as_const : std::false_type {};

  template <typename Result, typename... Cases>
  struct caches_cases_as_const<
      core::dsl::detail::on_cases<Result, Cases...>>
      : std::bool_constant<(
            is_stateless_handler_v<traits::case_handler_t<Cases>>
            && ...)> {};

  // The type a cache of `Cases` holds: const when no handler keeps
  // state, so the compiler can treat the cached cases as constants
  // (GCC lowers a dispatch on a non-const static to a jump table
  // instead of a lookup table), and mutable otherwise, so a stateful
  // handler's state persists between calls.
  template <typename Cases>
  using cached_cases_t = std::conditional_t<
      caches_cases_as_const<std::decay_t<Cases>>::value,
      const std::decay_t<Cases>,
      std::decay_t<Cases>>;

  // Builds the cases once, on first use, and returns the cached
  // object. Each factory type gets its own cache.
  template <typename Factory>
  PTN_DETAIL_ALWAYS_INLINE decltype(auto)
  cached_cases(Factory &&factory) {
    using factory_t = std::remove_reference_t<Factory>;
    using cases_t   = std::invoke_result_t<factory_t &>;
    static cached_cases_t<cases_t> cases = factory();
    return (cases);
  }

} // namespace ptn::core::engine::detail

namespace ptn {

  // Entry for pipeline matching.
  //
  // Lvalues are borrowed; rvalues are owned by the match context.
  // Usage:
  //   - match(value) | on(case1, case2, ..., _ >> fallback)
  template <typename T>
  constexpr auto match(T &&value) {
    using V = std::conditional_t<std::is_lvalue_reference_v<T>,
                                 T,
                                 std::remove_reference_t<T>>;
    return core::engine::detail::pipeline_match_context<V>::create(
        std::forward<T>(value));
  }

  // Caches an `on(...)` matcher behind a stateless factory.
  //
  // This is an explicit wrapper around the common handwritten
  // pattern: `static auto cases = on(...);`.
  //
  // Usage:
  //   - match(value) | static_on([] { return on(..., _ >>
  //   fallback); })
  //
  // The factory must be stateless so caching does not silently
  // freeze runtime captures on the first call.
  template <typename Factory>
  inline decltype(auto) static_on(Factory &&factory) {
    using factory_t = std::decay_t<Factory>;
    static_assert(
        std::is_empty_v<factory_t>,
        "[Patternia.static_on]: factory must be stateless. "
        "Tip: pass [] { return on(...); }.");
    static_assert(
        std::is_invocable_v<factory_t &>,
        "[Patternia.static_on]: factory must be invocable with no "
        "arguments.");

    using cases_t = std::decay_t<std::invoke_result_t<factory_t &>>;
    static_assert(
        core::dsl::detail::is_on_v<cases_t>,
        "[Patternia.static_on]: factory must return on(...).");

    return core::engine::detail::cached_cases(factory);
  }

} // namespace ptn
