#pragma once

#include "ptn/config.hpp"
#include <tuple>
#include <type_traits>
#include <utility>
#include <functional>

#if PTN_USE_CONCEPTS
#include <concepts>
#endif

#include "ptn/dsl/case_expr.hpp"
#if PTN_ENABLE_VALUE_PATTERN
#include "ptn/patterns/value.hpp"
#endif

namespace ptn {
  /* free match function forward declaration */
  template <typename T>
  constexpr auto
  match(T &&) noexcept(std::is_nothrow_constructible_v<std::decay_t<T>, T &&>);
} // namespace ptn

namespace ptn::detail {
#if PTN_USE_CONCEPTS
  template <typename H, typename X>
  concept Invocable1 = std::is_invocable_v<H &, X &>;
  template <typename H>
  concept Invocable0 = std::is_invocable_v<H &>;

  // handler callable with (x)
  template <typename H, typename X>
    requires Invocable1<H, X>
  constexpr decltype(auto) run_handler(H &h, X &x) {
    return std::invoke(h, x);
  }

  // not callable with (x) but callable with ()
  template <typename H, typename X>
    requires(!Invocable1<H, X> && Invocable0<H>)
  constexpr decltype(auto) run_handler(H &h, X & /*x*/) {
    return std::invoke(h);
  }

  // neither callable -> treat handler as plain value and return it
  template <typename H, typename X>
    requires(!Invocable1<H, X> && !Invocable0<H>)
  constexpr decltype(auto) run_handler(H &h, X & /*x*/) {
    return (h);
  }
#else
  // C++17 fallback: use SFINAE overloads
  template <
      typename H,
      typename X,
      typename = std::enable_if_t<std::is_invocable_v<H &, X &>>>
  constexpr decltype(auto) run_handler(H &h, X &x) {
    return std::invoke(h, x);
  }

  template <
      typename H,
      typename X,
      typename = void,
      typename = std::enable_if_t<
          !std::is_invocable_v<H &, X &> && std::is_invocable_v<H &>>>
  constexpr decltype(auto) run_handler(H &h, X & /*x*/) {
    return std::invoke(h);
  }

  template <
      typename H,
      typename X,
      typename = void,
      typename = void,
      typename = std::enable_if_t<
          !std::is_invocable_v<H &, X &> && !std::is_invocable_v<H &>>>
  constexpr decltype(auto) run_handler(H &h, X & /*x*/) {
    return (h);
  }
#endif
} // namespace ptn::detail

namespace ptn::core {
  struct ctor_tag {};

  template <typename TV, typename... Cases>
  class match_builder {
    TV                   value_;
    std::tuple<Cases...> cases_;
    using ctor_tag_t = ctor_tag;
    /* make all specializations of match_builder mutual friends  */
    template <typename, typename...>
    friend class match_builder;

    template <typename Tuple>
#if PTN_USE_CONCEPTS
      requires std::constructible_from<std::tuple<Cases...>, Tuple>
#endif
    explicit constexpr match_builder(TV &&v, Tuple &&cs, ctor_tag_t)
        : value_(std::forward<TV>(v)), cases_(std::forward<Tuple>(cs)) {
    }

  public:
    template <typename V, typename Tuple>
#if PTN_USE_CONCEPTS
      requires std::constructible_from<std::tuple<Cases...>, Tuple>
#endif
    static constexpr match_builder create(V &&v, Tuple &&cs) {
      return match_builder<TV, Cases...>(
          std::forward<V>(v), std::forward<Tuple>(cs), ctor_tag{});
    }
    // with
    template <typename Pattern, typename Handler>
    constexpr auto with(Pattern p, Handler h) & {
      using pair_t   = std::pair<Pattern, Handler>;
      auto new_cases = std::tuple_cat(
          cases_, std::make_tuple(pair_t{std::move(p), std::move(h)}));
      return match_builder<TV, Cases..., pair_t>(
          value_, std::move(new_cases), ctor_tag_t{});
    }

    template <typename Pattern, typename Handler>
    constexpr auto with(Pattern p, Handler h) && {
      using pair_t   = std::pair<Pattern, Handler>;
      auto new_cases = std::tuple_cat(
          std::move(cases_),
          std::make_tuple(pair_t{std::move(p), std::move(h)}));
      return match_builder<TV, Cases..., pair_t>(
          std::move(value_), std::move(new_cases), ctor_tag_t{});
    }

    // otherwise
    template <typename H>
    constexpr auto otherwise(H &&h) && {
      using R = std::common_type_t<
          decltype(ptn::detail::run_handler(
              std::declval<typename Cases::second_type &>(),
              std::declval<TV &>()))...,
          decltype(ptn::detail::run_handler(
              std::declval<std::decay_t<H> &>(), std::declval<TV &>()))>;
      R    out{};
      bool done    = false;
      auto try_one = [&](auto &c) {
        if (done)
          return;
        auto &[p, handler] = c;
        if (p(value_)) {
          out  = static_cast<R>(ptn::detail::run_handler(handler, value_));
          done = true;
        }
      };
      // unified iteration using std::apply + fold-expression (C++17)
      std::apply([&](auto &...cs) { (try_one(cs), ...); }, cases_);
      if (!done)
        out = static_cast<R>(ptn::detail::run_handler(h, value_));
      return out;
    }

#if PTN_ENABLE_VALUE_PATTERN
    // with_value
    template <class V, class H>
    constexpr auto with_value(V &&v, H &&h) & {
      using ptn::patterns::value;
      return with(value(std::forward<V>(v)), std::forward<H>(h));
    }

    template <class V, class H>
    constexpr auto with_value(V &&v, H &&h) && {
      using ptn::patterns::value;
      return std::move(*this).with(
          value(std::forward<V>(v)), std::forward<H>(h));
    }

    // with_value_cmp
    template <class V, class Cmp, class H>
    constexpr auto with_value_cmp(V &&v, Cmp &&cmp, H &&h) & {
      using store_t = ptn::patterns::value_store_t<V>;
      auto p        = ptn::patterns::value_pattern<store_t, std::decay_t<Cmp>>{
          store_t(std::forward<V>(v)), std::forward<Cmp>(cmp)};
      return with(std::move(p), std::forward<H>(h));
    }

    template <class V, class Cmp, class H>
    constexpr auto with_value_cmp(V &&v, Cmp &&cmp, H &&h) && {
      using store_t = ptn::patterns::value_store_t<V>;
      auto p        = ptn::patterns::value_pattern<store_t, std::decay_t<Cmp>>{
          store_t(std::forward<V>(v)), std::forward<Cmp>(cmp)};
      return std::move(*this).with(std::move(p), std::forward<H>(h));
    }
#endif

    // when
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