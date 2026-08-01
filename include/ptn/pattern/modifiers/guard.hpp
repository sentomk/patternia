#pragma once

// Guard modifiers and placeholder expressions for pattern matching.
//
// Provides guard functionality that combines patterns with predicate
// conditions. Includes placeholder expressions for comparison
// operations and the guarded_pattern wrapper for applying predicates
// to bound values.

#include <cstddef>
#include <cstdint>
#include <functional>
#include <type_traits>
#include <tuple>
#include <utility>

#include "ptn/pattern/base/fwd.h"
#include "ptn/pattern/base/pattern_base.hpp"
#include "ptn/pattern/base/pattern_traits.hpp"
#include "ptn/pattern/modifiers/placeholder.hpp"
#include "ptn/core/common/diagnostics.hpp"

namespace ptn::pat::mod {

  // Binary predicate template for comparison operations.
  template <typename Op, typename RHS>
  struct binary_predicate : traits::guard_predicate_tag {

    RHS rhs;

    template <typename T>
    constexpr bool operator()(T &&v) const
        noexcept(noexcept(Op{}(v, rhs))) {
      return Op{}(v, rhs);
    }
  };

  // Multi-value guard expressions

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
  //
  // The bound tuple is read through an lvalue on purpose: guard
  // evaluation is read-only, and forwarding the same tuple into
  // both operands would be unsequenced (bugprone-use-after-move).
  template <typename Tuple, typename Op, typename L, typename R>
  constexpr decltype(auto) eval(const bin_expr<Op, L, R> &e,
                                Tuple                   &&t) {
    return std::decay_t<Op>{}(eval(e.l, t), eval(e.r, t));
  }

  // Evaluates unary expression.
  template <typename Tuple, typename Op, typename X>
  constexpr decltype(auto) eval(const un_expr<Op, X> &e, Tuple &&t) {
    return Op{}(eval(e.x, std::forward<Tuple>(t)));
  }

  // Trap: member placeholders must be resolved to positional
  // placeholders by the structural pattern owning the guard. This
  // overload is only reached when a PTN_BIND name is used in a
  // guard on a non-structural pattern.
  template <auto M, typename Tuple>
  constexpr decltype(auto) eval(const member_t<M> &, Tuple &&) {
    static_assert(dependent_false<Tuple>::value,
                  "[Patternia.guard] A PTN_BIND member placeholder "
                  "was used in a guard on a non-structural pattern. "
                  "Member names are only valid in guards attached "
                  "to has<...>.");
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
    return tuple_predicate<std::decay_t<E>>{
        as_expr(std::forward<E>(e))};
  }

  template <std::size_t I>
  struct max_arg_index<arg_t<I>>
      : std::integral_constant<std::size_t, I> {};

  template <typename T>
  struct max_arg_index<val_t<T>>
      : std::integral_constant<std::size_t, 0> {};

  template <typename Op, typename L, typename R>
  struct max_arg_index<bin_expr<Op, L, R>>
      : std::integral_constant<std::size_t,
                               (max_arg_index<L>::value
                                        > max_arg_index<R>::value
                                    ? max_arg_index<L>::value
                                    : max_arg_index<R>::value)> {};

  template <typename Op, typename X>
  struct max_arg_index<un_expr<Op, X>>
      : std::integral_constant<std::size_t,
                               max_arg_index<X>::value> {};

  // Trap with a clear message when a member placeholder leaks into
  // the positional bounds check (non-structural guard).
  template <auto M>
  struct max_arg_index<member_t<M>> {
    static_assert(dependent_false<member_t<M>>::value,
                  "[Patternia.guard] A PTN_BIND member placeholder "
                  "was used in a guard on a non-structural pattern. "
                  "Member names are only valid in guards attached "
                  "to has<...>.");
    static constexpr std::size_t value = 0;
  };

  template <typename T>
  inline constexpr std::size_t
      max_arg_index_v = max_arg_index<std::decay_t<T>>::value;

  // Modulo operation helper.
  struct mod_op {
    template <typename A, typename B>
    constexpr decltype(auto) operator()(A &&a, B &&b) const {
      return std::forward<A>(a) % std::forward<B>(b);
    }
  };

  // --- operator overloads (arith / cmp / logic / unary) ---

  // Arithmetic operators.
  template <typename L,
            typename R,
            std::enable_if_t<(traits::is_arg_expr_v<L>
                              || traits::is_arg_expr_v<R>),
                             int> = 0>
  constexpr auto operator+(L &&l, R &&r) {
    return bin_expr<std::plus<>,
                    decltype(as_expr(std::forward<L>(l))),
                    decltype(as_expr(std::forward<R>(r)))>{
        as_expr(std::forward<L>(l)), as_expr(std::forward<R>(r))};
  }

  template <typename L,
            typename R,
            std::enable_if_t<(traits::is_arg_expr_v<L>
                              || traits::is_arg_expr_v<R>),
                             int> = 0>
  constexpr auto operator-(L &&l, R &&r) {
    return bin_expr<std::minus<>,
                    decltype(as_expr(std::forward<L>(l))),
                    decltype(as_expr(std::forward<R>(r)))>{
        as_expr(std::forward<L>(l)), as_expr(std::forward<R>(r))};
  }

  template <typename L,
            typename R,
            std::enable_if_t<(traits::is_arg_expr_v<L>
                              || traits::is_arg_expr_v<R>),
                             int> = 0>
  constexpr auto operator*(L &&l, R &&r) {
    return bin_expr<std::multiplies<>,
                    decltype(as_expr(std::forward<L>(l))),
                    decltype(as_expr(std::forward<R>(r)))>{
        as_expr(std::forward<L>(l)), as_expr(std::forward<R>(r))};
  }

  template <typename L,
            typename R,
            std::enable_if_t<(traits::is_arg_expr_v<L>
                              || traits::is_arg_expr_v<R>),
                             int> = 0>
  constexpr auto operator/(L &&l, R &&r) {
    return bin_expr<std::divides<>,
                    decltype(as_expr(std::forward<L>(l))),
                    decltype(as_expr(std::forward<R>(r)))>{
        as_expr(std::forward<L>(l)), as_expr(std::forward<R>(r))};
  }

  // Modulo operator.
  template <typename L,
            typename R,
            std::enable_if_t<(traits::is_arg_expr_v<L>
                              || traits::is_arg_expr_v<R>),
                             int> = 0>
  constexpr auto operator%(L &&l, R &&r) {
    return bin_expr<mod_op,
                    decltype(as_expr(std::forward<L>(l))),
                    decltype(as_expr(std::forward<R>(r)))>{
        as_expr(std::forward<L>(l)), as_expr(std::forward<R>(r))};
  }

  // Comparison operators returning tuple predicates
  template <typename L,
            typename R,
            std::enable_if_t<(traits::is_arg_expr_v<L>
                              || traits::is_arg_expr_v<R>),
                             int> = 0>
  constexpr auto operator==(L &&l, R &&r) {
    return make_pred(bin_expr<std::equal_to<>,
                              decltype(as_expr(std::forward<L>(l))),
                              decltype(as_expr(std::forward<R>(r)))>{
        as_expr(std::forward<L>(l)), as_expr(std::forward<R>(r))});
  }

  template <typename L,
            typename R,
            std::enable_if_t<(traits::is_arg_expr_v<L>
                              || traits::is_arg_expr_v<R>),
                             int> = 0>
  constexpr auto operator!=(L &&l, R &&r) {
    return make_pred(bin_expr<std::not_equal_to<>,
                              decltype(as_expr(std::forward<L>(l))),
                              decltype(as_expr(std::forward<R>(r)))>{
        as_expr(std::forward<L>(l)), as_expr(std::forward<R>(r))});
  }

  template <typename L,
            typename R,
            std::enable_if_t<(traits::is_arg_expr_v<L>
                              || traits::is_arg_expr_v<R>),
                             int> = 0>
  constexpr auto operator<(L &&l, R &&r) {
    return make_pred(bin_expr<std::less<>,
                              decltype(as_expr(std::forward<L>(l))),
                              decltype(as_expr(std::forward<R>(r)))>{
        as_expr(std::forward<L>(l)), as_expr(std::forward<R>(r))});
  }

  template <typename L,
            typename R,
            std::enable_if_t<(traits::is_arg_expr_v<L>
                              || traits::is_arg_expr_v<R>),
                             int> = 0>
  constexpr auto operator<=(L &&l, R &&r) {
    return make_pred(bin_expr<std::less_equal<>,
                              decltype(as_expr(std::forward<L>(l))),
                              decltype(as_expr(std::forward<R>(r)))>{
        as_expr(std::forward<L>(l)), as_expr(std::forward<R>(r))});
  }

  template <typename L,
            typename R,
            std::enable_if_t<(traits::is_arg_expr_v<L>
                              || traits::is_arg_expr_v<R>),
                             int> = 0>
  constexpr auto operator>(L &&l, R &&r) {
    return make_pred(bin_expr<std::greater<>,
                              decltype(as_expr(std::forward<L>(l))),
                              decltype(as_expr(std::forward<R>(r)))>{
        as_expr(std::forward<L>(l)), as_expr(std::forward<R>(r))});
  }

  template <typename L,
            typename R,
            std::enable_if_t<(traits::is_arg_expr_v<L>
                              || traits::is_arg_expr_v<R>),
                             int> = 0>
  constexpr auto operator>=(L &&l, R &&r) {
    return make_pred(bin_expr<std::greater_equal<>,
                              decltype(as_expr(std::forward<L>(l))),
                              decltype(as_expr(std::forward<R>(r)))>{
        as_expr(std::forward<L>(l)), as_expr(std::forward<R>(r))});
  }

  // Invokes a guard predicate with the given value.
  //
  // If the predicate is a guard_predicate_tag (tuple_predicate,
  // pred_and, etc.), it is called directly. Otherwise, if the value
  // is a tuple, it is unpacked via std::apply so that raw callables
  // receive individual elements.
  namespace detail {

    // Detects whether T is a std::tuple specialization.
    template <typename T>
    struct is_tuple : std::false_type {};

    template <typename... Ts>
    struct is_tuple<std::tuple<Ts...>> : std::true_type {};

    template <typename T>
    inline constexpr bool is_tuple_v = is_tuple<T>::value;

    template <typename Pred, typename Tuple, std::size_t... I>
    constexpr decltype(auto) invoke_from_tuple_impl(
        Pred &&pred, Tuple &&tuple, std::index_sequence<I...>) {
      return std::forward<Pred>(pred)(
          std::get<I>(std::forward<Tuple>(tuple))...);
    }

    template <typename Pred, typename Tuple>
    constexpr decltype(auto) invoke_from_tuple(Pred  &&pred,
                                               Tuple &&tuple) {
      using tuple_t = std::remove_reference_t<Tuple>;
      return invoke_from_tuple_impl(
          std::forward<Pred>(pred),
          std::forward<Tuple>(tuple),
          std::make_index_sequence<std::tuple_size_v<tuple_t>>{});
    }

    template <typename Pred, typename T>
    constexpr bool invoke_guard(const Pred &pred, T &&v) {
      if constexpr (std::is_base_of_v<traits::guard_predicate_tag,
                                      std::decay_t<Pred>>) {
        return static_cast<bool>(pred(std::forward<T>(v)));
      }
      else if constexpr (is_tuple_v<std::decay_t<T>>) {
        return static_cast<bool>(
            invoke_from_tuple(pred, std::forward<T>(v)));
      }
      else {
        return static_cast<bool>(pred(std::forward<T>(v)));
      }
    }

  } // namespace detail

  // Guard wrapper that makes a callable participate in guard
  // composition.
  template <typename Fn>
  struct callable_guard : traits::guard_predicate_tag {
    Fn fn;

    constexpr explicit callable_guard(Fn f) : fn(std::move(f)) {
    }

    template <typename... Args>
    constexpr bool operator()(Args &&...args) const {
      if constexpr (sizeof...(Args) == 1) {
        // guarded_pattern passes one tuple-like binding object here.
        return detail::invoke_guard(fn, std::forward<Args>(args)...);
      }
      else {
        // Direct multi-argument calls come from composed guard
        // predicates.
        return static_cast<bool>(fn(std::forward<Args>(args)...));
      }
    }
  };

  template <typename Fn>
  constexpr auto make_callable_guard(Fn &&fn) {
    return callable_guard<std::decay_t<Fn>>(std::forward<Fn>(fn));
  }

  // Logical AND for guard predicates.
  template <typename L, typename R>
  struct pred_and : traits::guard_predicate_tag {
    L lhs;
    R rhs;

    constexpr pred_and(L l, R r)
        : lhs(std::move(l)), rhs(std::move(r)) {
    }

    template <typename T>
    constexpr bool operator()(T &&v) const {
      return detail::invoke_guard(lhs, v)
             && detail::invoke_guard(rhs, v);
    }
  };

  // Logical OR for guard predicates.
  template <typename L, typename R>
  struct pred_or : traits::guard_predicate_tag {
    L lhs;
    R rhs;

    constexpr pred_or(L l, R r)
        : lhs(std::move(l)), rhs(std::move(r)) {
    }

    template <typename T>
    constexpr bool operator()(T &&v) const {
      return detail::invoke_guard(lhs, v)
             || detail::invoke_guard(rhs, v);
    }
  };

  // Computes maximum arg index used by a tuple guard predicate.
  template <typename T>
  struct max_tuple_guard_index
      : std::integral_constant<std::size_t, 0> {};

  template <typename E>
  struct max_tuple_guard_index<tuple_predicate<E>>
      : std::integral_constant<std::size_t, max_arg_index_v<E>> {};

  template <typename L, typename R>
  struct max_tuple_guard_index<pred_and<L, R>>
      : std::integral_constant<
            std::size_t,
            (max_tuple_guard_index<std::decay_t<L>>::value
                     > max_tuple_guard_index<std::decay_t<R>>::value
                 ? max_tuple_guard_index<std::decay_t<L>>::value
                 : max_tuple_guard_index<std::decay_t<R>>::value)> {
  };

  template <typename L, typename R>
  struct max_tuple_guard_index<pred_or<L, R>>
      : std::integral_constant<
            std::size_t,
            (max_tuple_guard_index<std::decay_t<L>>::value
                     > max_tuple_guard_index<std::decay_t<R>>::value
                 ? max_tuple_guard_index<std::decay_t<L>>::value
                 : max_tuple_guard_index<std::decay_t<R>>::value)> {
  };

  template <typename T>
  inline constexpr std::size_t
      max_tuple_guard_index_v = max_tuple_guard_index<
          std::decay_t<T>>::value;

  // Logical AND operator overload for guard predicates.
  template <typename L,
            typename R,
            std::enable_if_t<traits::is_guard_predicate_v<L>
                                 || traits::is_guard_predicate_v<R>,
                             int> = 0>
  constexpr auto operator&&(L &&l, R &&r) {
    return pred_and<std::decay_t<L>, std::decay_t<R>>{
        std::forward<L>(l), std::forward<R>(r)};
  }

  // Logical OR operator overload for guard predicates.
  template <typename L,
            typename R,
            std::enable_if_t<traits::is_guard_predicate_v<L>
                                 || traits::is_guard_predicate_v<R>,
                             int> = 0>
  constexpr auto operator||(L &&l, R &&r) {
    return pred_or<std::decay_t<L>, std::decay_t<R>>{
        std::forward<L>(l), std::forward<R>(r)};
  }

  // --- Member placeholder resolution (structural guards) ---

  namespace detail {

    // Type-safe member pointer equality: comparing member pointers
    // of different types is ill-formed, so guard the comparison.
    template <auto M, auto N, bool SameType>
    struct member_eq_impl : std::false_type {};

    template <auto M, auto N>
    struct member_eq_impl<M, N, true> : std::bool_constant<M == N> {
    };

    template <auto M, auto N>
    struct member_eq
        : member_eq_impl<
              M,
              N,
              (std::is_same_v<decltype(M), decltype(N)>)> {};

  } // namespace detail

  // Position of member M among the non-_ign members of Ms....
  // _ign (nullptr) slots do not occupy binding positions.
  template <auto M, auto... Ms>
  struct member_position;

  template <auto M>
  struct member_position<M> {
    static constexpr bool        found = false;
    static constexpr std::size_t value = 0;
  };

  template <auto M, auto First, auto... Rest>
  struct member_position<M, First, Rest...> {
  private:
    static constexpr bool
        is_ign = std::is_null_pointer_v<decltype(First)>;
    static constexpr bool
        hit    = !is_ign && detail::member_eq<M, First>::value;
    using next = member_position<M, Rest...>;

  public:
    static constexpr bool        found = hit || next::found;
    static constexpr std::size_t value = hit ? 0
                                             : (is_ign ? next::value
                                                       : next::value
                                                             + 1);
  };

  // Rewrites a guard expression tree, replacing every member_t<M>
  // leaf with arg_t<position of M in Ms...>. All other nodes are
  // preserved. Evaluation machinery is untouched.
  //
  // Fallback: non-expression nodes pass through unchanged (by
  // value, so stored types stay decayed).
  template <auto... Ms, typename E>
  constexpr auto resolve_expr(member_list<Ms...>, E &&e) {
    return std::forward<E>(e);
  }

  template <auto... Ms, auto M>
  constexpr auto resolve_expr(member_list<Ms...>, member_t<M>) {
    using pos = member_position<M, Ms...>;
    static_assert(pos::found,
                  "[Patternia.guard] A PTN_BIND name does not match "
                  "any member listed in has<...>. Check the member "
                  "pointers in the pattern.");
    return arg_t<pos::value>{};
  }

  template <auto... Ms, typename Op, typename L, typename R>
  constexpr auto resolve_expr(member_list<Ms...> ml,
                              bin_expr<Op, L, R> e) {
    auto l = resolve_expr(ml, e.l);
    auto r = resolve_expr(ml, e.r);
    return bin_expr<Op, decltype(l), decltype(r)>{std::move(l),
                                                  std::move(r)};
  }

  template <auto... Ms, typename Op, typename X>
  constexpr auto resolve_expr(member_list<Ms...> ml,
                              un_expr<Op, X>     e) {
    auto x = resolve_expr(ml, e.x);
    return un_expr<Op, decltype(x)>{std::move(x)};
  }

  // Rewrites a full guard predicate. Expression predicates
  // (tuple_predicate) and logical compositions (pred_and/pred_or)
  // are rewritten; callables and other predicates pass through.
  template <auto... Ms, typename P>
  constexpr auto resolve_pred(member_list<Ms...>, P &&p) {
    return std::forward<P>(p);
  }

  template <auto... Ms, typename E>
  constexpr auto resolve_pred(member_list<Ms...> ml,
                              tuple_predicate<E> p) {
    auto e = resolve_expr(ml, std::move(p.expr));
    return tuple_predicate<decltype(e)>{std::move(e)};
  }

  template <auto... Ms, typename L, typename R>
  constexpr auto resolve_pred(member_list<Ms...> ml,
                              pred_and<L, R>     p) {
    auto l = resolve_pred(ml, std::move(p.lhs));
    auto r = resolve_pred(ml, std::move(p.rhs));
    return pred_and<decltype(l), decltype(r)>{std::move(l),
                                              std::move(r)};
  }

  template <auto... Ms, typename L, typename R>
  constexpr auto resolve_pred(member_list<Ms...> ml,
                              pred_or<L, R>      p) {
    auto l = resolve_pred(ml, std::move(p.lhs));
    auto r = resolve_pred(ml, std::move(p.rhs));
    return pred_or<decltype(l), decltype(r)>{std::move(l),
                                             std::move(r)};
  }

  // Range modes for interval predicates.
  enum class range_mode : std::uint8_t {
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
    return range_predicate<std::decay_t<T>>{{},
                                            std::forward<T>(lo),
                                            std::forward<T>(hi),
                                            range_mode::closed};
  }

  // Creates open range predicate (lo, hi).
  template <typename T>
  constexpr auto rng(T lo, T hi, open_t) {
    return range_predicate<std::decay_t<T>>{{},
                                            std::forward<T>(lo),
                                            std::forward<T>(hi),
                                            range_mode::open};
  }

  // Creates open-closed range predicate (lo, hi].
  template <typename T>
  constexpr auto rng(T lo, T hi, open_closed_t) {
    return range_predicate<std::decay_t<T>>{{},
                                            std::forward<T>(lo),
                                            std::forward<T>(hi),
                                            range_mode::open_closed};
  }

  // Creates closed-open range predicate [lo, hi).
  template <typename T>
  constexpr auto rng(T lo, T hi, closed_open_t) {
    return range_predicate<std::decay_t<T>>{{},
                                            std::forward<T>(lo),
                                            std::forward<T>(hi),
                                            range_mode::closed_open};
  }

  // Pattern wrapper that applies a predicate guard to an inner
  // pattern.
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
    constexpr bool match(Subject &&s) const
        noexcept(noexcept(inner.match(std::forward<Subject>(s)))
                 && noexcept(inner.bind(std::forward<Subject>(s)))) {

      if (!inner.match(std::forward<Subject>(s)))
        return false;

      auto bound    = inner.bind(std::forward<Subject>(s));
      using bound_t = std::decay_t<decltype(bound)>;

      constexpr std::size_t N = std::tuple_size_v<bound_t>;

      if constexpr (ptn::pat::traits::is_tuple_guard_predicate_v<
                        Pred>) {
        // compile-time bounds check: max arg index must be < N
        ptn::core::common::static_assert_tuple_guard_index<
            ptn::pat::mod::max_tuple_guard_index_v<Pred>,
            N>();

        return static_cast<bool>(
            pred(bound)); // tuple-level predicate
      }
      else {
        // Callable guard: lambda or custom functor.
        // Also handles binary_predicate and range_predicate via
        // tuple expansion for single-element bindings.
        return static_cast<bool>(
            detail::invoke_from_tuple(pred, bound));
      }
    }

    // Binds subject using inner pattern's binding logic.
    template <typename Subject>
    constexpr decltype(auto) bind(Subject &&subject) const noexcept(
        noexcept(inner.bind(std::forward<Subject>(subject)))) {
      return inner.bind(std::forward<Subject>(subject));
    }
  };

} // namespace ptn::pat::mod

namespace ptn::pat::base {

  // Guards on already-guarded patterns delegate member-placeholder
  // resolution to the inner pattern (chained guard support).
  template <typename Inner, typename Pred>
  struct guard_resolver<mod::guarded_pattern<Inner, Pred>> {
    template <typename P>
    static constexpr auto apply(P &&pred) {
      return guard_resolver<Inner>::apply(std::forward<P>(pred));
    }
  };

  // Binding contract specialization for guarded_pattern.
  template <typename Inner, typename Pred, typename Subject>
  struct binding_args<mod::guarded_pattern<Inner, Pred>, Subject> {
    using type = typename binding_args<Inner, Subject>::type;
  };
} // namespace ptn::pat::base
