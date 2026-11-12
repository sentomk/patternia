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
          auto bound = std::forward_as_tuple(args...);
          if constexpr (ValueExpression) {
            // Materialize the result before a pattern's temporary
            // binding tuple is destroyed by the invocation layer.
            return pat::mod::eval(predicate.expr, bound);
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
  };

  template <typename Pattern, typename Expression>
  constexpr auto make_expression_handler(Expression &&expression) {
    constexpr bool
        value_expression = pat::traits::is_arg_expr_v<Expression>;
    if constexpr (value_expression) {
      auto resolved = pat::base::guard_resolver<Pattern>::apply(
          pat::mod::make_pred(std::forward<Expression>(expression)));
      return expression_handler<decltype(resolved), true>{
          std::move(resolved)};
    }
    else {
      auto resolved = pat::base::guard_resolver<Pattern>::apply(
          std::forward<Expression>(expression));
      return expression_handler<decltype(resolved), false>{
          std::move(resolved)};
    }
  }

} // namespace ptn::core::dsl::detail
