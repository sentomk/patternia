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
  // free match function forward declaration
  template<typename U>
  constexpr auto match(U &&) noexcept(std::is_nothrow_constructible_v<std::decay_t<U>, U &&>);
} // namespace ptn

namespace ptn::detail {
#if PTN_USE_CONCEPTS
  template<typename H, typename X>
  concept Invocable1 = std::is_invocable_v<H &, X &>;
  template<typename H>
  concept Invocable0 = std::is_invocable_v<H &>;
#else
  template<typename H, typename X>
  using Invocable1 = std::is_invocable<H &, X &>;
  template<typename H>
  using Invocable0 = std::is_invocable<H &>;
#endif

  template<typename H, typename X>
  constexpr decltype(auto) run_handler(H &h, X &x) {
#if PTN_USE_CONCEPTS
    if constexpr (Invocable1<H, X>) {
      return std::invoke(h, x);
    } else if constexpr (Invocable0<H>) {
      return std::invoke(h);
    } else {
      return (h);
    }
#else
    return std::invoke(h, x);
#endif
  }
} // namespace ptn::detail

namespace ptn::core {
  struct ctor_tag {
  };

  template<typename TV, typename... Cases>
  class match_builder {
    TV value_;
    std::tuple<Cases...> cases_;
    using ctor_tag_t = ctor_tag;

    template<typename U>
    friend constexpr auto ::ptn::match(U &&) noexcept(std::is_nothrow_constructible_v<std::decay_t<U>, U &&>);

    template<typename Tuple>
#if PTN_USE_CONCEPTS
    requires std::constructible_from<std::tuple<Cases...>, Tuple>
#endif
    explicit constexpr match_builder(TV &&v, Tuple &&cs, ctor_tag_t)
      : value_(std::forward<TV>(v)), cases_(std::forward<Tuple>(cs)) {
    }

  public:
    // with
    template<typename Pattern, typename Handler>
    constexpr auto with(Pattern p, Handler h) & {
      using pair_t = std::pair<Pattern, Handler>;
      auto new_cases = std::tuple_cat(cases_, std::make_tuple(pair_t{std::move(p), std::move(h)}));
      return match_builder<TV, Cases..., pair_t>(value_, std::move(new_cases), ctor_tag_t{});
    }

    template<typename Pattern, typename Handler>
    constexpr auto with(Pattern p, Handler h) && {
      using pair_t = std::pair<Pattern, Handler>;
      auto new_cases = std::tuple_cat(std::move(cases_), std::make_tuple(pair_t{std::move(p), std::move(h)}));
      return match_builder<TV, Cases..., pair_t>(std::move(value_), std::move(new_cases), ctor_tag_t{});
    }

    // otherwise
    template<typename H>
    constexpr auto otherwise(H &&h) && {
      using R = std::common_type_t<
        decltype(ptn::detail::run_handler(std::declval<typename Cases::second_type &>(), std::declval<TV &>()))...,
        decltype(ptn::detail::run_handler(std::declval<std::decay_t<H> &>(), std::declval<TV &>()))>;
      R out{};
      bool done = false;
      auto try_one = [&](auto &c) {
        if (done) return;
        auto &[p, handler] = c;
        if (p(value_)) {
          out = static_cast<R>(ptn::detail::run_handler(handler, value_));
          done = true;
        }
      };
      // unified iteration using std::apply + fold-expression (C++17)
      std::apply([&](auto &... cs) { (try_one(cs), ...); }, cases_);
      if (!done) out = static_cast<R>(ptn::detail::run_handler(h, value_));
      return out;
    }

#if PTN_ENABLE_VALUE_PATTERN
    // with_value
    template<class V, class H>
    constexpr auto with_value(V &&v, H &&h) & {
      using ptn::patterns::value;
      return with(value(std::forward<V>(v)), std::forward<H>(h));
    }

    template<class V, class H>
    constexpr auto with_value(V &&v, H &&h) && {
      using ptn::patterns::value;
      return std::move(*this).with(value(std::forward<V>(v)), std::forward<H>(h));
    }

    // with_value_cmp
    template<class V, class Cmp, class H>
    constexpr auto with_value_cmp(V &&v, Cmp &&cmp, H &&h) & {
      using store_t = ptn::patterns::value_store_t<V>;
      auto p = ptn::patterns::value_pattern<store_t, std::decay_t<Cmp> >{
        store_t(std::forward<V>(v)), std::forward<Cmp>(cmp)
      };
      return with(std::move(p), std::forward<H>(h));
    }

    template<class V, class Cmp, class H>
    constexpr auto with_value_cmp(V &&v, Cmp &&cmp, H &&h) && {
      using store_t = ptn::patterns::value_store_t<V>;
      auto p = ptn::patterns::value_pattern<store_t, std::decay_t<Cmp> >{
        store_t(std::forward<V>(v)), std::forward<Cmp>(cmp)
      };
      return std::move(*this).with(std::move(p), std::forward<H>(h));
    }
#endif

    // when
    template<typename Pattern, typename Handler>
    constexpr auto when(dsl::case_expr<Pattern, Handler> &&e) & {
      return this->with(std::move(e.pattern), std::move(e.handler));
    }

    template<typename Pattern, typename Handler>
    constexpr auto when(dsl::case_expr<Pattern, Handler> &&e) && {
      return std::move(*this).with(std::move(e.pattern), std::move(e.handler));
    }
  };
} // namespace ptn::core