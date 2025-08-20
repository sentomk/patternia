#ifndef PTN_MATCH_BUILDER_HPP
#define PTN_MATCH_BUILDER_HPP

#include <tuple>
#include <concepts>
#include <type_traits>
#include <utility>

#include "ptn/dsl/case_expr.hpp"

#include "ptn/config.hpp"
#if PTN_ENABLE_VALUE_PATTERN
#include "ptn/patterns/value.hpp"
#endif

/* namespace ptn */
namespace ptn {
  /* grant friend access to free function match() so it can call hidden ctor */
  template <typename U>
  constexpr auto
  match(U &&) noexcept(std::is_nothrow_constructible_v<std::decay_t<U>, U &&>);
} // namespace ptn

/* namespace ptn::detail */
namespace ptn::detail {
  template <typename H, typename X>
  concept Invocable1 = std::is_invocable_v<H &, X &>;

  template <typename H>
  concept Invocable0 = std::is_invocable_v<H &>;

  template <typename H, typename X>
  constexpr decltype(auto) run_handler(H &h, X &x) {
    if constexpr (Invocable1<H, X>) {
      return std::invoke(h, x);
    }
    else if constexpr (Invocable0<H>) {
      return std::invoke(h);
    }
    else {
      return (h);
    }
  }
} // namespace ptn::detail

/* namespace ptn::core */
namespace ptn::core {

  /* private tag type used to hide ctor. only friends can pass ctor_tag{} */
  struct ctor_tag {};

  /* class match_builder start */
  template <typename T, typename... Cases>
  class match_builder {

    using ctor_tag = ptn::core::ctor_tag;

    template <typename U>
    friend constexpr auto ::ptn::match(U &&) noexcept(
        std::is_nothrow_constructible_v<std::decay_t<U>, U &&>);

    /* make all specializations of match_builder mutual friends */
    template <typename, typename...>
    friend class match_builder;

    /* member variables*/
    T                    value_;
    std::tuple<Cases...> cases_;

    /* perfect forwarding ctor */
    template <class TV, class Tuple>
    requires std::constructible_from<std::tuple<Cases...>, Tuple>
    explicit constexpr match_builder(TV &&v, Tuple &&cs, ctor_tag)
        : value_(std::forward<TV>(v)), cases_(std::forward<Tuple>(cs)) {
    }

  public:
    /* return itself or a new builder. only can be called on lvalue */
    template <typename Pattern, typename Handler>
    constexpr auto with(Pattern p, Handler h) & {
      using pair_t   = std::pair<Pattern, Handler>;
      auto new_cases = std::tuple_cat(
          cases_, std::make_tuple(pair_t{std::move(p), std::move(h)}));
      return match_builder<T, Cases..., pair_t>(
          value_, std::move(new_cases), ctor_tag{});
    }

    /* overload for "with", now it supports rvalue */
    template <typename Pattern, typename Handler>
    constexpr auto with(Pattern p, Handler h) && {
      using pair_t   = std::pair<Pattern, Handler>;
      auto new_cases = std::tuple_cat(
          std::move(cases_),
          std::make_tuple(pair_t{std::move(p), std::move(h)}));
      return match_builder<T, Cases..., pair_t>(
          std::move(value_), std::move(new_cases), ctor_tag{});
    }

    /* register a default branch and triggers match execution. only for rvalue
     */
    template <typename H>
    constexpr auto otherwise(H &&h) && {
      using R = std::common_type_t<
          decltype(ptn::detail::run_handler(
              std::declval<typename Cases::second_type &>(),
              std::declval<T &>()))...,
          decltype(ptn::detail::run_handler(
              std::declval<std::decay_t<H> &>(), std::declval<T &>()))>;

      R    out{};
      bool done = false;

      auto try_one = [&](auto &c) {
        if (done)
          return;
        auto &[p, handler] = c;
        if (p(value_)) {
          out  = static_cast<R>(ptn::detail::run_handler(handler, value_));
          done = true;
        }
      };

      std::apply([&](auto &...cs) { (try_one(cs), ...); }, cases_);

      if (!done)
        out = static_cast<R>(ptn::detail::run_handler(h, value_));
      return out;
    }

#if PTN_ENABLE_VALUE_PATTERN

    /* value pattern: with_value(v, h) == with(value(v), h)*/
    template <class V, class H>
    constexpr auto with_value(V &&v, H &&h) & {
      using ptn::patterns::value;
      return with(value(std::forward<V>(v)), std::forward<H>(h));
    }

    /* overload of with_value for rval */
    template <typename Value, typename Handler>
    constexpr auto with_value(Value &&v, Handler &&h) && {
      using ptn::patterns::value;
      return std::move(*this).with(
          value(std::forward<Value>(v)), std::forward<Handler>(h));
    }

    /* allows specifying a "per-branch" custom comparator for value matching */
    template <class V, class Cmp, class H>
    constexpr auto with_value_cmp(V &&v, Cmp &&cmp, H &&h) & {
      using store_t = ptn::patterns::value_store_t<V>;
      auto p        = ptn::patterns::value_pattern<store_t, std::decay_t<Cmp>>{
                 store_t(std::forward<V>(v)), std::forward<Cmp>(cmp)};
      return with(std::move(p), std::forward<H>(h));
    }

    /* overload of with_value_cmp for rvalue match_builder */
    template <class V, class Cmp, class H>
    constexpr auto with_value_cmp(V &&v, Cmp &&cmp, H &&h) && {
      using store_t = ptn::patterns::value_store_t<V>;
      auto p        = ptn::patterns::value_pattern<store_t, std::decay_t<Cmp>>{
                 store_t(std::forward<V>(v)), std::forward<Cmp>(cmp)};
      return std::move(*this).with(std::move(p), std::forward<H>(h));
    }
#endif

    /* for match(x).when(p >> "h") */
    template <typename Pattern, typename Handler>
    constexpr auto when(dsl::case_expr<Pattern, Handler> &&e) & {
      return this->with(std::move(e.pattern), std::move(e.handler));
    }

    template <typename Pattern, typename Handler>
    constexpr auto when(dsl::case_expr<Pattern, Handler> &&e) && {
      return std::move(*this).with(std::move(e.pattern), std::move(e.handler));
    }
  };

} // namespace ptn::core

#endif // PTN_MATCH_BUILDER_HPP