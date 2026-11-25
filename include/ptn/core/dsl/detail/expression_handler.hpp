#pragma once

#include <tuple>
#include <type_traits>
#include <utility>

#include "ptn/pattern/modifiers/guard.hpp"

namespace ptn::core::dsl::detail {

  // Adapt guard expressions to the ordinary bound-argument handler
  // contract. Arithmetic expressions retain their result type;
  // predicates retain their bool result and short-circuit behavior.
  template <typename Predicate, bool ValueExpression>
  struct expression_handler {
    Predicate predicate;

    // Do not expose a nullary overload: expressions need bindings,
    // and handler classification also probes nullary invocability.
    template <typename... Args,
              std::enable_if_t<(sizeof...(Args) > 0), int> = 0>
    constexpr auto operator()(Args &&...args) const {
      if constexpr (pat::traits::is_tuple_guard_predicate_v<
                        Predicate>) {
        constexpr bool
            in_range = pat::mod::max_tuple_guard_index_v<Predicate>
                       < sizeof...(Args);
        static_assert(
            in_range,
            "[Patternia.handler]: placeholder is out of range "
            "for the pattern's bound values.");
        if constexpr (in_range) {
          auto bound = std::forward_as_tuple(
              std::forward<Args>(args)...);
          if constexpr (ValueExpression) {
            // Materialize the result before a pattern's temporary
            // binding tuple is destroyed by the invocation layer.
            return pat::mod::eval(predicate.expr, std::move(bound));
          }
          else {
            return predicate(bound);
          }
        }
        else {
          // Keep the bounds diagnostic from cascading into std::get.
          return 0;
        }
      }
      else {
        // Unary predicates such as rng use the same calling
        // convention as guards, receiving the bound value directly.
        return predicate(std::forward<Args>(args)...);
      }
    }

    // The result for `case | f`. `operator()` returns by value
    // because its result can outlive the bindings; here the result
    // goes straight to `f` within the same call, while the bindings
    // are alive. A bare placeholder such as `_` or a member name
    // therefore stays a reference to the bound value, so
    // `$(is<T>) >> _ | f` does not copy the alternative before
    // calling `f`. Other expressions produce a value either way.
    template <typename... Args,
              std::enable_if_t<(sizeof...(Args) > 0), int> = 0>
    constexpr decltype(auto) pipe_source(Args &&...args) const {
      if constexpr (ValueExpression
                    && pat::traits::is_tuple_guard_predicate_v<
                        Predicate>) {
        if constexpr (pat::mod::max_tuple_guard_index_v<Predicate>
                      < sizeof...(Args)) {
          return pat::mod::eval(
              predicate.expr,
              std::forward_as_tuple(std::forward<Args>(args)...));
        }
        else {
          // operator() reports the out-of-range placeholder.
          return (*this)(std::forward<Args>(args)...);
        }
      }
      else {
        return (*this)(std::forward<Args>(args)...);
      }
    }
  };

  template <typename Handler>
  inline constexpr bool is_expression_handler_v = false;

  template <typename Predicate, bool ValueExpression>
  inline constexpr bool is_expression_handler_v<
      expression_handler<Predicate, ValueExpression>> = true;

  // Stands in for an expression handler whose member names are
  // unbound, once that has been reported. It accepts any bindings,
  // so the handler-signature check does not add a second error.
  struct unbound_member_handler {
    template <typename... Args>
    constexpr int operator()(Args &&...) const {
      return 0;
    }
  };

  // Resolves member names in an expression or predicate against
  // the case's pattern. A function rather than a lambda: MSVC
  // instantiates both `if constexpr` branches of a lambda whose
  // return type is deduced.
  template <typename Pattern,
            bool ValueExpression,
            typename Expression>
  constexpr auto resolve_expression(Expression &&expression) {
    if constexpr (ValueExpression) {
      return pat::base::guard_resolver<Pattern>::apply(
          pat::mod::make_pred(std::forward<Expression>(expression)));
    }
    else {
      return pat::base::guard_resolver<Pattern>::apply(
          std::forward<Expression>(expression));
    }
  }

  template <typename Pattern, typename Expression>
  constexpr auto make_expression_handler(Expression &&expression) {
    constexpr bool
         value_expression = pat::traits::is_arg_expr_v<Expression>;
    auto resolved = resolve_expression<Pattern, value_expression>(
        std::forward<Expression>(expression));
    using R = std::decay_t<decltype(resolved)>;
    // Member names that the pattern did not resolve. The usual
    // cause is $ around is<T>(...) or alt<I>(...), which binds the
    // whole alternative rather than the members of a has<...>
    // inside it.
    static_assert(
        !pat::mod::contains_member_v<R>,
        "[Patternia.handler]: A member name in this handler is not "
        "bound by the case's pattern. Member names are bound only "
        "by $(has<...>); $(is<T>(has<...>)) binds the whole "
        "alternative, not its members. Tip: write "
        "is<T>($(has<...>)) or alt<I>($(has<...>)) to bind the "
        "members.");
    if constexpr (pat::mod::contains_member_v<R>) {
      return unbound_member_handler{};
    }
    else {
      return expression_handler<R, value_expression>{
          std::move(resolved)};
    }
  }

} // namespace ptn::core::dsl::detail
