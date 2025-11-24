#pragma once

#include <type_traits>
#include <utility>

namespace ptn::dsl::detail {
  template <typename P, typename H>
  struct case_expr;
}

namespace ptn::pattern::value {

  template <typename F>
  struct predicate_pattern;

} // namespace ptn::pattern::value

namespace ptn::pattern::value::detail {

  template <typename L, typename R>
  struct and_pattern;

  template <typename L, typename R>
  struct or_pattern;

  template <typename P>
  struct not_pattern;

} // namespace ptn::pattern::value::detail

namespace ptn::pattern::detail {
  template <typename T>
  struct is_pattern;

  template <typename P>
  extern const bool is_pattern_v;
} // namespace ptn::pattern::detail

namespace ptn::dsl::ops {

  // ========== operator>> (pattern >> handler) ==========
  template <typename P, typename H>
  constexpr auto operator>>(P &&p, H &&h) {
    return ptn::dsl::detail::case_expr<std::decay_t<P>, std::decay_t<H>>{
        std::forward<P>(p), std::forward<H>(h)};
  }

  // =====================================================
  // Logical DSL operators: && || !
  // =====================================================

#if defined(PTN_USE_CONCEPTS) && PTN_USE_CONCEPTS

  template <ptn::pattern_like L, ptn::pattern_like R>
  constexpr auto operator&&(L &&l, R &&r) {
    return ptn::value::and_pattern<std::decay_t<L>, std::decay_t<R>>(
        std::forward<L>(l), std::forward<R>(r));
  }

  template <ptn::pattern_like L, ptn::pattern_like R>
  constexpr auto operator||(L &&l, R &&r) {
    return ptn::value::or_pattern<std::decay_t<L>, std::decay_t<R>>(
        std::forward<L>(l), std::forward<R>(r));
  }

  template <ptn::pattern_like P>
  constexpr auto operator!(P &&p) {
    return ptn::value::not_pattern<std::decay_t<P>>(std::forward<P>(p));
  }

#else // C++17 fallback (SFINAE)

  template <
      typename L,
      typename R,
      typename = std::enable_if_t<
          pattern::detail::is_pattern_v<std::decay_t<L>> &&
          pattern::detail::is_pattern_v<std::decay_t<R>>>>
  constexpr auto operator&&(L &&l, R &&r) {
    return ptn::pattern::value::detail::
        and_pattern<std::decay_t<L>, std::decay_t<R>>(
            std::forward<L>(l), std::forward<R>(r));
  }

  template <
      typename L,
      typename R,
      typename = std::enable_if_t<
          pattern::detail::is_pattern_v<std::decay_t<L>> &&
          pattern::detail::is_pattern_v<std::decay_t<R>>>>
  constexpr auto operator||(L &&l, R &&r) {
    return ptn::pattern::value::detail::
        or_pattern<std::decay_t<L>, std::decay_t<R>>(
            std::forward<L>(l), std::forward<R>(r));
  }

  template <
      typename P,
      typename =
          std::enable_if_t<pattern::detail::is_pattern_v<std::decay_t<P>>>>
  constexpr auto operator!(P &&p) {
    return ptn::pattern::value::detail::not_pattern<std::decay_t<P>>(
        std::forward<P>(p));
  }

#endif

} // namespace ptn::dsl::ops