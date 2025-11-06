#include <functional>
#include <type_traits>
#include <utility>
#include "ptn/config.hpp"

namespace ptn::patterns {

  template <typename F>
  struct predicate_pattern {
#if defined(__cpp_no_unique_address) && __cpp_no_unique_address >= 201803L
    [[no_unique_address]] F fn;
#else
    F fn;
#endif
    template <typename X>
    constexpr bool operator()(X const &x) const
        noexcept(noexcept(std::is_nothrow_invocable_v<const F &, X>)) {
      return std::invoke(fn, x);
    }
  };

  // Factory
  template <typename F>
  constexpr auto pred(F &&f) {
    return predicate_pattern<std::decay_t<F>>{std::forward<F>(f)};
  }

  /* logical composition */
  template <typename L, typename R>
  struct and_pattern {
    L l;
    R r;
    template <typename X>
    constexpr bool operator()(X const &x) const
        noexcept(noexcept(l(x) && r(x))) {
      return l(x) && r(x);
    }
  };
  template <typename L, typename R>
  struct or_pattern {
    L l;
    R r;
    template <typename X>
    constexpr bool operator()(X const &x) const
        noexcept(noexcept(l(x) || r(x))) {
      return l(x) || r(x);
    }
  };
  template <typename P>
  struct not_pattern {
    P p;
    template <typename X>
    constexpr bool operator()(X const &x) const noexcept(noexcept(!p(x))) {
      return !p(x);
    }
  };

#if PTN_USE_CONCEPTS
  template <typename P>
  concept pattern_like = requires(P p, int x) {
    { p(x) } -> std::convertible_to<bool>;
  };
#endif

  /* Logical operators (guarded) */

  namespace ops {
#if PTN_USE_CONCEPTS // C++ 20

    template <pattern_like L, pattern_like R>
    constexpr auto operator&&(L l, R r) {
      return and_pattern<L, R>{std::move(l), std::move(r)};
    }

    template <pattern_like L, pattern_like R>
    constexpr auto operator||(L l, R r) {
      return or_pattern<L, R>{std::move(l), std::move(r)};
    }

    template <pattern_like P>
    constexpr auto operator!(P p) {
      return not_pattern<P>{std::move(p)};
    }

#else // C++ 17

    template <
        typename L,
        typename R,
        typename = std::enable_if_t<
            std::is_invocable_r_v<bool, L, int> &&
            std::is_invocable_r_v<bool, R, int>>>
    constexpr auto operator&&(L l, R r) {
      return and_pattern<L, R>{std::move(l), std::move(r)};
    }

    template <
        typename L,
        typename R,
        typename = std::enable_if_t<
            std::is_invocable_r_v<bool, L, int> &&
            std::is_invocable_r_v<bool, R, int>>>
    constexpr auto operator||(L l, R r) {
      return or_pattern<L, R>{std::move(l), std::move(r)};
    }

    template <
        typename P,
        typename = std::enable_if_t<std::is_invocable_r_v<bool, P, int>>>
    constexpr auto operator!(P p) {
      return not_pattern<P>{std::move(p)};
    }

#endif

  } // namespace ops

} // namespace ptn::patterns