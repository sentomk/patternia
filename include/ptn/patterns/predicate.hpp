#pragma once

#include <type_traits>
#include <utility>
#include "ptn/patterns/pattern_tag.hpp"
#include "ptn/config.hpp"

namespace ptn::patterns {

  /* predicate_pattern: wraps any callable returning bool */
  template <typename F>
  struct predicate_pattern : pattern_tag {
#if defined(__cpp_no_unique_address) && __cpp_no_unique_address >= 201803L
    [[no_unique_address]] F fn;
#else
    F fn;
#endif

    constexpr explicit predicate_pattern(F f) : fn(std::move(f)) {
    }

    template <typename X>
    constexpr bool operator()(X const &x) const
        noexcept(noexcept(std::invoke(std::declval<const F &>(), x))) {
      return std::invoke(fn, x);
    }
  };

  // Factory
  template <typename F>
  constexpr auto pred(F &&f) {
    return predicate_pattern<std::decay_t<F>>(std::forward<F>(f));
  }

  /* logical composition */

  // and
  template <typename L, typename R>
  struct and_pattern : pattern_tag {
#if defined(__cpp_no_unique_address) && __cpp_no_unique_address >= 201803L
    [[no_unique_address]] L l;
    [[no_unique_address]] R r;
#else
    L l;
    R r;
#endif

    constexpr and_pattern(L lhs, R rhs) : l(std::move(lhs)), r(std::move(rhs)) {
    }
    template <typename X>
    constexpr bool operator()(X const &x) const
        noexcept(noexcept(l(x) && noexcept(r(x)))) {
      return l(x) && r(x);
    }
  };

  // or
  template <typename L, typename R>
  struct or_pattern : pattern_tag {
#if defined(__cpp_no_unique_address) && __cpp_no_unique_address >= 201803L
    [[no_unique_address]] L l;
    [[no_unique_address]] R r;
#else
    L l;
    R r;
#endif

    constexpr or_pattern(L lhs, R rhs) : l(std::move(lhs)), r(std::move(rhs)) {
    }
    template <typename X>
    constexpr bool operator()(X const &x) const
        noexcept(noexcept(l(x) && noexcept(r(x)))) {
      return l(x) || r(x);
    }
  };

  // not
  template <typename P>
  struct not_pattern : pattern_tag {
#if defined(__cpp_no_unique_address) && __cpp_no_unique_address >= 201803L
    [[no_unique_address]] P p;
#else
    P p;
#endif

    constexpr explicit not_pattern(P inner) : p(std::move(inner)) {
    }
    template <typename X>
    constexpr bool operator()(X const &x) const noexcept(noexcept(!p(x))) {
      return !p(x);
    }
  };

  /* ops namespace */
  namespace ops {

#if defined(__cpp_concepts) && __cpp_concepts >= 201907L
    template <pattern_like L, pattern_like R>
    constexpr auto operator&&(L &&l, R &&r) {
      return and_pattern<std::decay_t<L>, std::decay_t<R>>(
          std::forward<L>(l), std::forward<R>(r));
    }

    template <pattern_like L, pattern_like R>
    constexpr auto operator||(L &&l, R &&r) {
      return or_pattern<std::decay_t<L>, std::decay_t<R>>(
          std::forward<L>(l), std::forward<R>(r));
    }

    template <pattern_like P>
    constexpr auto operator!(P &&p) {
      return not_pattern<std::decay_t<P>>(std::forward<P>(p));
    }

#else
    template <
        typename L,
        typename R,
        typename = std::enable_if_t<
            std::is_base_of_v<pattern_tag, std::decay_t<L>> &&
            std::is_base_of_v<pattern_tag, std::decay_t<R>>>>
    constexpr auto operator&&(L &&l, R &&r) {
      return and_pattern<std::decay_t<L>, std::decay_t<R>>(
          std::forward<L>(l), std::forward<R>(r));
    }

    template <
        typename L,
        typename R,
        typename = std::enable_if_t<
            std::is_base_of_v<pattern_tag, std::decay_t<L>> &&
            std::is_base_of_v<pattern_tag, std::decay_t<R>>>>
    constexpr auto operator||(L &&l, R &&r) {
      return or_pattern<std::decay_t<L>, std::decay_t<R>>(
          std::forward<L>(l), std::forward<R>(r));
    }

    template <
        typename P,
        typename =
            std::enable_if_t<std::is_base_of_v<pattern_tag, std::decay_t<P>>>>
    constexpr auto operator!(P &&p) {
      return not_pattern<std::decay_t<P>>(std::forward<P>(p));
    }
#endif

  } // namespace ops

} // namespace ptn::patterns