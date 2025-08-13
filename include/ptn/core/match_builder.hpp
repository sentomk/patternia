#ifndef MATCH_BUILDER_HPP
#define MATCH_BUILDER_HPP

#include <tuple>
#include <concepts>
#include <type_traits>
#include <utility>

#include "ptn/patterns/value.hpp"

/* namespace ptn */
namespace ptn {
  /* grant friend access to free function match() so it can call hidden ctor */
  template <typename U>
  constexpr auto match(U &&) noexcept(std::is_nothrow_constructible_v<std::decay_t<U>, U &&>);
} // namespace ptn

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
      auto new_cases = std::tuple_cat(cases_, std::make_tuple(pair_t{std::move(p), std::move(h)}));
      return match_builder<T, Cases..., pair_t>(value_, std::move(new_cases), ctor_tag{});
    }

    /* overload for "with", now it supports rvalue */
    template <typename Pattern, typename Handler>
    constexpr auto with(Pattern p, Handler h) && {
      using pair_t = std::pair<Pattern, Handler>;
      auto new_cases =
          std::tuple_cat(std::move(cases_), std::make_tuple(pair_t{std::move(p), std::move(h)}));
      return match_builder<T, Cases..., pair_t>(
          std::move(value_), std::move(new_cases), ctor_tag{});
    }

    /* register a “default branch” and triggers match execution. only for rvalue */
    template <typename Handler>
    constexpr auto otherwise(Handler h) && {
      using R = std::common_type_t<std::invoke_result_t<Handler, T &&>,
                                   std::invoke_result_t<typename Cases::second_type, T &&>...>;
      R    result{};
      bool matched = false;

      /* if matched */
      auto try_case = [&](auto &&case_pair) {
        if (!matched && case_pair.first(value_)) {
          result  = case_pair.second(std::move(value_));
          matched = true;
        }
      };

      std::apply([&](auto &&...cs) { (try_case(cs), ...); }, cases_);

      /* if not matched */
      if (!matched) {
        result = h(std::move(value_));
      }
      return result;
    }

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
      return std::move(*this).with(value(std::forward<Value>(v)), std::forward<Handler>(h));
    }

    /* allows specifying a "per-branch" custom comparator for value matching */
    template <class V, class Cmp, class H>
    constexpr auto with_value_cmp(V &&v, Cmp &&cmp, H &&h) & {
      using store_t = ptn::patterns::value_store_t<V>;
      auto p = ptn::patterns::value_pattern<store_t, std::decay_t<Cmp>>{store_t(std::forward<V>(v)),
                                                                        std::forward<Cmp>(cmp)};
      return with(std::move(p), std::forward<H>(h));
    }

    /* overload of with_value_cmp for rvalue match_builder */
    template <class V, class Cmp, class H>
    constexpr auto with_value_cmp(V &&v, Cmp &&cmp, H &&h) && {
      using store_t = ptn::patterns::value_store_t<V>;
      auto p = ptn::patterns::value_pattern<store_t, std::decay_t<Cmp>>{store_t(std::forward<V>(v)),
                                                                        std::forward<Cmp>(cmp)};
      return std::move(*this).with(std::move(p), std::forward<H>(h));
    }
  };

} // namespace ptn::core
#endif // MATCH_BUILDER_HPP