#pragma once

#include "ptn/config.hpp"
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>
#include <functional>

#if PTN_USE_CONCEPTS
#include <concepts>
#endif

/**
 * @file match_builder_impl.hpp (detail)
 * @brief Internal implementation of the core matching engine.
 *
 * This is an internal header containing the implementation of:
 * - `match_builder<TV, Cases...>`: Core engine storing subject and case list.
 * - `run_handler()`: Handler invocation logic.
 *
 * Users should use the public `match()` function and `.when()/.otherwise()` API
 * from `ptn/core/match_builder.hpp` instead of directly instantiating
 * match_builder.
 *
 * Part of the Core Layer (namespace ptn::core::detail)
 *
 * @internal This is an internal implementation detail.
 */

namespace ptn::dsl::detail {
  template <typename Pattern, typename Handler>
  struct case_expr;
}

namespace ptn::core::detail {

#if PTN_USE_CONCEPTS

  template <typename H, typename X>
  concept Invocable1 = std::is_invocable_v<H &, X &>;

  template <typename H>
  concept Invocable0 = std::is_invocable_v<H &>;

  /**
   * @brief Invoke handler `h(x)` if available.
   *
   * @internal This is an internal implementation detail.
   */
  template <typename H, typename X>
    requires Invocable1<H, X>
  constexpr decltype(auto) run_handler(H &h, X &x) {
    return std::invoke(h, x);
  }

  /**
   * @brief Invoke handler `h()` if `(x)` overload is not available.
   *
   * @internal This is an internal implementation detail.
   */
  template <typename H, typename X>
    requires(!Invocable1<H, X> && Invocable0<H>)
  constexpr decltype(auto) run_handler(H &h, X &) {
    return std::invoke(h);
  }

  /**
   * @brief Fallback: return handler as value.
   *
   * @internal This is an internal implementation detail.
   */
  template <typename H, typename X>
    requires(!Invocable1<H, X> && !Invocable0<H>)
  constexpr decltype(auto) run_handler(H &h, X &) {
    return (h);
  }

#else // C++17 fallback

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
  constexpr decltype(auto) run_handler(H &h, X &) {
    return std::invoke(h);
  }

  template <
      typename H,
      typename X,
      typename = void,
      typename = void,
      typename = std::enable_if_t<
          !std::is_invocable_v<H &, X &> && !std::is_invocable_v<H &>>>
  constexpr decltype(auto) run_handler(H &h, X &) {
    return (h);
  }

#endif

  /**
   * @brief Tag used to disambiguate private constructors.
   *
   * @internal This is an internal implementation detail.
   */
  struct ctor_tag {};

  /**
   * @brief Core engine storing subject and case list, evaluated via
   * `.otherwise()`.
   *
   * @tparam TV     Type of stored subject value.
   * @tparam Cases  Variadic pack of `(Pattern, Handler)` pairs.
   *
   * This class is immutable: each `.when()` returns a new type with the added
   * case.
   *
   * @internal This is an internal implementation detail. Use `match()`
   * function.
   */
  template <typename TV, typename... Cases>
  class match_builder {
    TV                   value_; ///< Subject value
    std::tuple<Cases...> cases_; ///< Stored (Pattern, Handler) pairs
    using ctor_tag_t = ptn::core::detail::ctor_tag;

    template <typename P, typename H>
    friend struct ptn::dsl::detail::case_expr;

    template <typename, typename...>
    friend class match_builder;

    /**
     * @brief Private constructor used by `create()`.
     */
    template <typename TV2, typename Tuple>
#if PTN_USE_CONCEPTS
      requires std::constructible_from<TV, TV2 &&> &&
                   std::constructible_from<std::tuple<Cases...>, Tuple>
#endif
    explicit constexpr match_builder(TV2 &&v, Tuple &&cs, ctor_tag_t)
        : value_(std::forward<TV2>(v)), cases_(std::forward<Tuple>(cs)) {
    }

    // Append a case (lvalue)
    template <typename Pattern, typename Handler>
    constexpr auto with(Pattern p, Handler h) & {
      using pair_t   = std::pair<Pattern, Handler>;
      auto new_cases = std::tuple_cat(
          cases_, std::make_tuple(pair_t{std::move(p), std::move(h)}));

      return match_builder<TV, Cases..., pair_t>(
          value_, std::move(new_cases), ctor_tag_t{});
    }

    // Append a case (rvalue)
    template <typename Pattern, typename Handler>
    constexpr auto with(Pattern p, Handler h) && {
      using pair_t   = std::pair<Pattern, Handler>;
      auto new_cases = std::tuple_cat(
          std::move(cases_),
          std::make_tuple(pair_t{std::move(p), std::move(h)}));

      return match_builder<TV, Cases..., pair_t>(
          std::move(value_), std::move(new_cases), ctor_tag_t{});
    }

    // Recursive case matching
    /**
     * @brief Try each case in order.
     *
     * Stops when a pattern matches or all cases exhausted.
     *
     * @tparam I     Current tuple index.
     * @tparam OutT  Output type inferred from handlers.
     */
    template <std::size_t I = 0, typename OutT>
    constexpr void try_cases(OutT &out, bool &done) {
      if constexpr (I < sizeof...(Cases)) {
        auto &c            = std::get<I>(cases_);
        auto &[p, handler] = c;

        // Unified Pattern Layer API: Pattern must implement match(value)
        if (!done && p.match(value_)) {
          out  = static_cast<OutT>(run_handler(handler, value_));
          done = true;
        }
        else {
          try_cases<I + 1>(out, done);
        }
      }
    }

  public:
    // Builder creation (called from ptn::match())
    template <typename VArg, typename Tuple>
#if PTN_USE_CONCEPTS
      requires std::constructible_from<std::tuple<Cases...>, Tuple>
#endif
    static constexpr auto create(VArg &&v, Tuple &&cs)
        -> match_builder<std::decay_t<VArg>, Cases...> {
      using result_t = match_builder<std::decay_t<VArg>, Cases...>;
      return result_t(
          std::forward<VArg>(v), std::forward<Tuple>(cs), ctor_tag{});
    }

    // otherwise()
    /**
     * @brief Finalize matching.
     *
     * Computes a common return type from all handlers, applies the first
     * matching handler, or the fallback if none match.
     */
    template <typename H>
    constexpr auto otherwise(H &&fallback) && {
      using R = std::common_type_t<
          decltype(run_handler(
              std::declval<typename Cases::second_type &>(),
              std::declval<TV &>()))...,
          decltype(run_handler(
              std::declval<std::decay_t<H> &>(), std::declval<TV &>()))>;

      R    out{};
      bool done = false;

      try_cases(out, done);

      if (!done)
        out = static_cast<R>(run_handler(fallback, value_));

      return out;
    }

    // when()
    /**
     * @brief Add a pattern-handler case from a `case_expr`.
     */
    template <typename Pattern, typename Handler>
    constexpr auto when(ptn::dsl::detail::case_expr<Pattern, Handler> &&e) & {
      return this->with(std::move(e.pattern), std::move(e.handler));
    }

    template <typename Pattern, typename Handler>
    constexpr auto when(ptn::dsl::detail::case_expr<Pattern, Handler> &&e) && {
      return std::move(*this).with(std::move(e.pattern), std::move(e.handler));
    }
  };

} // namespace ptn::core::detail
