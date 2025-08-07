#ifndef PATTERNIA_HPP
#define PATTERNIA_HPP

#include <tuple>
#include <type_traits>
#include <utility>

namespace ptn {

  // forward declare
  template <typename T>
  constexpr auto match(T &&) noexcept;

  /* class match_builder start */
  template <typename T, typename... Cases>
  class match_builder {
    friend constexpr auto match(T &&) noexcept;

  private:
    T                    value_;
    std::tuple<Cases...> cases_;

    /* Perfect forwarding conductor. */
    explicit constexpr match_builder(T v, std::tuple<Cases...> cs)
        : value_(std::move(v)), cases_(cs) {
    }

  public:
    /* Return itself or a new builder. Only can be called on lvalue */
    template <typename Pattern, typename Handler>
    constexpr auto with(Pattern p, Handler h) & {
      using pair_t   = std::pair<Pattern, Handler>;
      auto new_cases = std::tuple_cat(cases_, std::make_tuple(pair_t{std::move(p), std::move(h)}));
      return match_builder<T, Cases..., pair_t>(std::move(value_), std::move(new_cases));
    }

    /* Overload for "with", now it supports rvalue. */
    template <typename Pattern, typename Handler>
    constexpr auto with(Pattern p, Handler h) && {
      using pair_t = std::pair<Pattern, Handler>;
      auto new_cases =
          std::tuple_cat(std::move(cases_), std::make_tuple(pair_t{std::move(p), std::move(h)}));
      return match_builder<T, Cases..., pair_t>(std::move(value_), std::move(new_cases));
    }

    /* Register a “default branch” and triggers match execution. Only for rvalue.*/
    template <typename Handler>
    constexpr auto otherwise(Handler h) && {
      using R =
          std::common_type_t<std::invoke_result_t<Handler, T>,
                             std::invoke_result_t<decltype(std::get<Cases>(cases_).second), T>...>;
      R    result{};
      bool matched = false;

      std::apply(
          [&](auto &...casePair) {
            (([&] {
               if (!matched && casePair.first(value_)) {
                 result  = casePair.second(std::move(value_));
                 matched = true;
               }
             }()),
             ...);
          },
          cases_);

      if (!matched) {
        result = h(std::move(value_));
      }
      return result;
    }
  }; // class match_builder

  template <typename T>
  constexpr auto match(T &&value) noexcept {
    using V = std::decay_t<T>;
    return match_builder<V>(std::forward<T>(value), std::tuple<>());
  }
} // namespace ptn

#endif // PATTERNIA_HPP