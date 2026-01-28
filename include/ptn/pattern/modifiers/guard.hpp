#pragma once

// Guard modifiers and placeholder expressions for pattern matching.
//
// Provides guard functionality that combines patterns with predicate
// conditions. Includes placeholder expressions for comparison operations
// and the guarded_pattern wrapper for applying predicates to bound values.

#include <cstddef>
#include <functional>
#include <type_traits>
#include <tuple>
#include <utility>

#include "ptn/pattern/base/fwd.h"
#include "ptn/pattern/base/pattern_base.hpp"
#include "ptn/pattern/base/pattern_traits.hpp"
#include "ptn/core/common/diagnostics.hpp"

namespace ptn::pat::mod {

  // Binary predicate template for comparison operations.
  template <typename Op, typename RHS>
  struct binary_predicate : traits::guard_predicate_tag {

    RHS rhs;

    template <typename T>
    constexpr bool operator()(T &&v) const noexcept(noexcept(Op{}(v, rhs))) {
      return Op{}(v, rhs);
    }
  };

  // Placeholder type for creating comparison expressions.
  struct placeholder_t {

    // Greater than comparison.
    template <typename T>
    auto operator>(T &&rhs) const {
      return binary_predicate<std::greater<>, std::decay_t<T>>{
          {}, std::forward<T>(rhs)};
    }

    // Less than comparison.
    template <typename T>
    auto operator<(T &&rhs) const {
      return binary_predicate<std::less<>, std::decay_t<T>>{
          {}, std::forward<T>(rhs)};
    }

    // Greater than or equal comparison.
    template <typename T>
    auto operator>=(T &&rhs) const {
      return binary_predicate<std::greater_equal<>, std::decay_t<T>>{
          {}, std::forward<T>(rhs)};
    }

    // Less than or equal comparison.
    template <typename T>
    auto operator<=(T &&rhs) const {
      return binary_predicate<std::less_equal<>, std::decay_t<T>>{
          {}, std::forward<T>(rhs)};
    }

    // Equality comparison.
    template <typename T>
    auto operator==(T &&rhs) const {
      return binary_predicate<std::equal_to<>, std::decay_t<T>>{
          {}, std::forward<T>(rhs)};
    }

    // Inequality comparison.
    template <typename T>
    auto operator!=(T &&rhs) const {
      return binary_predicate<std::not_equal_to<>, std::decay_t<T>>{
          {}, std::forward<T>(rhs)};
    }
  };

  // Global placeholder instance for guard expressions.
  inline constexpr placeholder_t _{};

  // Multi-value guard expressions
  //
  // arg<N>: placeholder for the Nth bound element.
  template <std::size_t I>
  struct arg_t {
    static constexpr std::size_t index = I;
  };

  // Argument placeholder for multi-value expressions.
  template <std::size_t I>
  inline constexpr arg_t<I> arg{};

  // Value wrapper for literals in expression templates.
  template <typename T>
  struct val_t {
    T v;
  };

  // Creates a value wrapper for expression templates.
  template <typename T>
  constexpr auto val(T &&x) {
    return val_t<std::decay_t<T>>{std::forward<T>(x)};
  }

  // Binary expression node for expression templates.
  template <typename Op, typename L, typename R>
  struct bin_expr {
    L l;
    R r;
  };

  // Unary expression node for expression templates.
  template <typename Op, typename X>
  struct un_expr {
    X x;
  };

  // Normalizes operands to expression nodes.
  template <typename T>
  constexpr auto as_expr(T &&x) {
    if constexpr (traits::is_arg_expr_v<T>) {
      return std::forward<T>(x);
    }
    else {
      return val(std::forward<T>(x));
    }
  }

  // Evaluates argument placeholder against bound tuple.
  template <typename Tuple, std::size_t I>
  constexpr decltype(auto) eval(const arg_t<I> &, Tuple &&t) {
    return std::get<I>(std::forward<Tuple>(t));
  }

  // Evaluates value wrapper.
  template <typename Tuple, typename T>
  constexpr decltype(auto) eval(const val_t<T> &v, Tuple &&) {
    return (v.v);
  }

  // Evaluates binary expression.
  template <typename Tuple, typename Op, typename L, typename R>
  constexpr decltype(auto) eval(const bin_expr<Op, L, R> &e, Tuple &&t) {
    return std::decay_t<Op>{}(
        eval(e.l, std::forward<Tuple>(t)), eval(e.r, std::forward<Tuple>(t)));
  }

  // Evaluates unary expression.
  template <typename Tuple, typename Op, typename X>
  constexpr decltype(auto) eval(const un_expr<Op, X> &e, Tuple &&t) {
    return Op{}(eval(e.x, std::forward<Tuple>(t)));
  }

  // Makes expression callable as predicate on bound tuples.
  template <typename Expr>
  struct tuple_predicate : traits::guard_predicate_tag {
    Expr expr;

    constexpr explicit tuple_predicate(Expr e) : expr(std::move(e)) {
    }

    template <typename Tuple>
    constexpr bool operator()(Tuple &&t) const {
      return eval(expr, std::forward<Tuple>(t));
    }
  };

  // Creates a tuple predicate from expression.
  template <typename E>
  constexpr auto make_pred(E &&e) {
    return tuple_predicate<std::decay_t<E>>{as_expr(std::forward<E>(e))};
  }

  // Computes maximum argument index in expression.
  template <typename E>
  struct max_arg_index;

  template <std::size_t I>
  struct max_arg_index<arg_t<I>> : std::integral_constant<std::size_t, I> {};

  template <typename T>
  struct max_arg_index<val_t<T>> : std::integral_constant<std::size_t, 0> {};

  template <typename Op, typename L, typename R>
  struct max_arg_index<bin_expr<Op, L, R>>
      : std::integral_constant<
            std::size_t,
            (max_arg_index<L>::value > max_arg_index<R>::value
                 ? max_arg_index<L>::value
                 : max_arg_index<R>::value)> {};

  template <typename Op, typename X>
  struct max_arg_index<un_expr<Op, X>>
      : std::integral_constant<std::size_t, max_arg_index<X>::value> {};

  template <typename T>
  inline constexpr std::size_t max_arg_index_v =
      max_arg_index<std::decay_t<T>>::value;

  // Modulo operation helper.
  struct mod_op {
    template <typename A, typename B>
    constexpr decltype(auto) operator()(A &&a, B &&b) const {
      return std::forward<A>(a) % std::forward<B>(b);
    }
  };

  // --- operator overloads (arith / cmp / logic / unary) ---

  // Arithmetic operators
  template <
      typename L,
      typename R,
      std::enable_if_t<
          (traits::is_arg_expr_v<L> || traits::is_arg_expr_v<R>),
          int> = 0>
  constexpr auto operator+(L &&l, R &&r) {
    return bin_expr<
        std::plus<>,
        decltype(as_expr(std::forward<L>(l))),
        decltype(as_expr(std::forward<R>(r)))>{
        as_expr(std::forward<L>(l)), as_expr(std::forward<R>(r))};
  }

  template <
      typename L,
      typename R,
      std::enable_if_t<
          (traits::is_arg_expr_v<L> || traits::is_arg_expr_v<R>),
          int> = 0>
  constexpr auto operator-(L &&l, R &&r) {
    return bin_expr<
        std::minus<>,
        decltype(as_expr(std::forward<L>(l))),
        decltype(as_expr(std::forward<R>(r)))>{
        as_expr(std::forward<L>(l)), as_expr(std::forward<R>(r))};
  }

  template <
      typename L,
      typename R,
      std::enable_if_t<
          (traits::is_arg_expr_v<L> || traits::is_arg_expr_v<R>),
          int> = 0>
  constexpr auto operator*(L &&l, R &&r) {
    return bin_expr<
        std::multiplies<>,
        decltype(as_expr(std::forward<L>(l))),
        decltype(as_expr(std::forward<R>(r)))>{
        as_expr(std::forward<L>(l)), as_expr(std::forward<R>(r))};
  }

  template <
      typename L,
      typename R,
      std::enable_if_t<
          (traits::is_arg_expr_v<L> || traits::is_arg_expr_v<R>),
          int> = 0>
  constexpr auto operator/(L &&l, R &&r) {
    return bin_expr<
        std::divides<>,
        decltype(as_expr(std::forward<L>(l))),
        decltype(as_expr(std::forward<R>(r)))>{
        as_expr(std::forward<L>(l)), as_expr(std::forward<R>(r))};
  }

  // Modulo operator
  template <
      typename L,
      typename R,
      std::enable_if_t<
          (traits::is_arg_expr_v<L> || traits::is_arg_expr_v<R>),
          int> = 0>
  constexpr auto operator%(L &&l, R &&r) {
    return bin_expr<
        mod_op,
        decltype(as_expr(std::forward<L>(l))),
        decltype(as_expr(std::forward<R>(r)))>{
        as_expr(std::forward<L>(l)), as_expr(std::forward<R>(r))};
  }

  // Comparison operators returning tuple predicates
  template <
      typename L,
      typename R,
      std::enable_if_t<
          (traits::is_arg_expr_v<L> || traits::is_arg_expr_v<R>),
          int> = 0>
  constexpr auto operator==(L &&l, R &&r) {
    return make_pred(
        bin_expr<
            std::equal_to<>,
            decltype(as_expr(std::forward<L>(l))),
            decltype(as_expr(std::forward<R>(r)))>{
            as_expr(std::forward<L>(l)), as_expr(std::forward<R>(r))});
  }

  template <
      typename L,
      typename R,
      std::enable_if_t<
          (traits::is_arg_expr_v<L> || traits::is_arg_expr_v<R>),
          int> = 0>
  constexpr auto operator!=(L &&l, R &&r) {
    return make_pred(
        bin_expr<
            std::not_equal_to<>,
            decltype(as_expr(std::forward<L>(l))),
            decltype(as_expr(std::forward<R>(r)))>{
            as_expr(std::forward<L>(l)), as_expr(std::forward<R>(r))});
  }

  template <
      typename L,
      typename R,
      std::enable_if_t<
          (traits::is_arg_expr_v<L> || traits::is_arg_expr_v<R>),
          int> = 0>
  constexpr auto operator<(L &&l, R &&r) {
    return make_pred(
        bin_expr<
            std::less<>,
            decltype(as_expr(std::forward<L>(l))),
            decltype(as_expr(std::forward<R>(r)))>{
            as_expr(std::forward<L>(l)), as_expr(std::forward<R>(r))});
  }

  template <
      typename L,
      typename R,
      std::enable_if_t<
          (traits::is_arg_expr_v<L> || traits::is_arg_expr_v<R>),
          int> = 0>
  constexpr auto operator<=(L &&l, R &&r) {
    return make_pred(
        bin_expr<
            std::less_equal<>,
            decltype(as_expr(std::forward<L>(l))),
            decltype(as_expr(std::forward<R>(r)))>{
            as_expr(std::forward<L>(l)), as_expr(std::forward<R>(r))});
  }

  template <
      typename L,
      typename R,
      std::enable_if_t<
          (traits::is_arg_expr_v<L> || traits::is_arg_expr_v<R>),
          int> = 0>
  constexpr auto operator>(L &&l, R &&r) {
    return make_pred(
        bin_expr<
            std::greater<>,
            decltype(as_expr(std::forward<L>(l))),
            decltype(as_expr(std::forward<R>(r)))>{
            as_expr(std::forward<L>(l)), as_expr(std::forward<R>(r))});
  }

  template <
      typename L,
      typename R,
      std::enable_if_t<
          (traits::is_arg_expr_v<L> || traits::is_arg_expr_v<R>),
          int> = 0>
  constexpr auto operator>=(L &&l, R &&r) {
    return make_pred(
        bin_expr<
            std::greater_equal<>,
            decltype(as_expr(std::forward<L>(l))),
            decltype(as_expr(std::forward<R>(r)))>{
            as_expr(std::forward<L>(l)), as_expr(std::forward<R>(r))});
  }

  // Logical AND for guard predicates.
  template <typename L, typename R>
  struct pred_and : traits::guard_predicate_tag {
    L lhs;
    R rhs;

    constexpr pred_and(L l, R r) : lhs(std::move(l)), rhs(std::move(r)) {
    }

    template <typename T>
    constexpr bool operator()(T &&v) const {
      return lhs(v) && rhs(v);
    }
  };

  // Logical OR for guard predicates.
  template <typename L, typename R>
  struct pred_or : traits::guard_predicate_tag {
    L lhs;
    R rhs;

    constexpr pred_or(L l, R r) : lhs(std::move(l)), rhs(std::move(r)) {
    }

    template <typename T>
    constexpr bool operator()(T &&v) const {
      return lhs(v) || rhs(v);
    }
  };

  // Computes maximum arg index used by a tuple guard predicate.
  template <typename T>
  struct max_tuple_guard_index : std::integral_constant<std::size_t, 0> {};

  template <typename E>
  struct max_tuple_guard_index<tuple_predicate<E>>
      : std::integral_constant<std::size_t, max_arg_index_v<E>> {};

  template <typename L, typename R>
  struct max_tuple_guard_index<pred_and<L, R>>
      : std::integral_constant<
            std::size_t,
            (max_tuple_guard_index<std::decay_t<L>>::value >
                     max_tuple_guard_index<std::decay_t<R>>::value
                 ? max_tuple_guard_index<std::decay_t<L>>::value
                 : max_tuple_guard_index<std::decay_t<R>>::value)> {};

  template <typename L, typename R>
  struct max_tuple_guard_index<pred_or<L, R>>
      : std::integral_constant<
            std::size_t,
            (max_tuple_guard_index<std::decay_t<L>>::value >
                     max_tuple_guard_index<std::decay_t<R>>::value
                 ? max_tuple_guard_index<std::decay_t<L>>::value
                 : max_tuple_guard_index<std::decay_t<R>>::value)> {};

  template <typename T>
  inline constexpr std::size_t max_tuple_guard_index_v =
      max_tuple_guard_index<std::decay_t<T>>::value;

  // Logical AND operator overload for guard predicates.
  template <
      typename L,
      typename R,
      std::enable_if_t<
          traits::is_guard_predicate_v<L> || traits::is_guard_predicate_v<R>,
          int> = 0>
  constexpr auto operator&&(L &&l, R &&r) {
    return pred_and<std::decay_t<L>, std::decay_t<R>>{
        std::forward<L>(l), std::forward<R>(r)};
  }

  // Logical OR operator overload for guard predicates.
  template <
      typename L,
      typename R,
      std::enable_if_t<
          traits::is_guard_predicate_v<L> || traits::is_guard_predicate_v<R>,
          int> = 0>
  constexpr auto operator||(L &&l, R &&r) {
    return pred_or<std::decay_t<L>, std::decay_t<R>>{
        std::forward<L>(l), std::forward<R>(r)};
  }

  // Range modes for interval predicates.
  enum class range_mode {
    closed,
    open,
    open_closed,
    closed_open
  };

  // Range mode type tags.
  struct closed_t {};
  struct open_t {};
  struct open_closed_t {};
  struct closed_open_t {};

  // Range mode constants.
  inline constexpr closed_t      closed{};
  inline constexpr open_t        open{};
  inline constexpr open_closed_t open_closed{};
  inline constexpr closed_open_t closed_open{};

  // Range predicate for interval checking.
  template <typename T>
  struct range_predicate : traits::guard_predicate_tag {
    T          lo;
    T          hi;
    range_mode mode;

    template <typename U>
    constexpr bool operator()(U &&v) const {
      switch (mode) {
      case range_mode::closed:
        return lo <= v && v <= hi;
      case range_mode::open:
        return lo < v && v < hi;
      case range_mode::open_closed:
        return lo < v && v <= hi;
      case range_mode::closed_open:
        return lo <= v && v < hi;
      }
      return false;
    }
  };

  // Creates closed range predicate [lo, hi].
  template <typename T>
  constexpr auto rng(T lo, T hi) {
    return range_predicate<std::decay_t<T>>{
        {}, std::forward<T>(lo), std::forward<T>(hi), range_mode::closed};
  }

  // Creates open range predicate (lo, hi).
  template <typename T>
  constexpr auto rng(T lo, T hi, open_t) {
    return range_predicate<std::decay_t<T>>{
        {}, std::forward<T>(lo), std::forward<T>(hi), range_mode::open};
  }

  // Creates open-closed range predicate (lo, hi].
  template <typename T>
  constexpr auto rng(T lo, T hi, open_closed_t) {
    return range_predicate<std::decay_t<T>>{
        {}, std::forward<T>(lo), std::forward<T>(hi), range_mode::open_closed};
  }

  // Creates closed-open range predicate [lo, hi).
  template <typename T>
  constexpr auto rng(T lo, T hi, closed_open_t) {
    return range_predicate<std::decay_t<T>>{
        {}, std::forward<T>(lo), std::forward<T>(hi), range_mode::closed_open};
  }

  // Pattern wrapper that applies a predicate guard to an inner pattern.
  template <typename Inner, typename Pred>
  struct guarded_pattern
      : base::pattern_base<guarded_pattern<Inner, Pred>>,
        base::binding_pattern_base<guarded_pattern<Inner, Pred>> {
    Inner inner;
    Pred  pred;

    // Constructs guarded pattern with inner pattern and predicate.
    template <typename I, typename P>
    constexpr guarded_pattern(I &&i, P &&p)
        : inner(std::forward<I>(i)), pred(std::forward<P>(p)) {
    }

    // Attempts to match subject against guarded pattern.
    template <typename Subject>
    constexpr bool match(Subject &&s) const noexcept(
        noexcept(inner.match(std::forward<Subject>(s))) &&
        noexcept(inner.bind(std::forward<Subject>(s)))) {

      if (!inner.match(std::forward<Subject>(s)))
        return false;

      auto bound    = inner.bind(std::forward<Subject>(s));
      using bound_t = std::decay_t<decltype(bound)>;

      constexpr std::size_t N = std::tuple_size_v<bound_t>;

      if constexpr (ptn::pat::traits::is_tuple_guard_predicate_v<Pred>) {
        // compile-time bounds check: max arg index must be < N
        ptn::core::common::static_assert_tuple_guard_index<
            ptn::pat::mod::max_tuple_guard_index_v<Pred>,
            N>();

        return static_cast<bool>(pred(bound)); // tuple-level predicate
      }
      else if constexpr (ptn::pat::traits::is_guard_predicate_v<Pred>) {
        ptn::core::common::static_assert_unary_guard_arity<N>();

        return static_cast<bool>(pred(std::get<0>(bound)));
      }
      else {
        // Callable guard: lambda / where / custom functor
        return static_cast<bool>(std::apply(pred, bound));
      }
    }

    // Binds subject using inner pattern's binding logic.
    template <typename Subject>
    constexpr decltype(auto) bind(Subject &&subject) const
        noexcept(noexcept(inner.bind(std::forward<Subject>(subject)))) {
      return inner.bind(std::forward<Subject>(subject));
    }
  };

} // namespace ptn::pat::mod

namespace ptn::pat::base {

  // Binding contract specialization for guarded_pattern.
  template <typename Inner, typename Pred, typename Subject>
  struct binding_args<mod::guarded_pattern<Inner, Pred>, Subject> {
    using type = typename binding_args<Inner, Subject>::type;
  };
} // namespace ptn::pat::base
