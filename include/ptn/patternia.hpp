#ifndef PATTERNIA_HPP
#define PATTERNIA_HPP

#include <tuple>
#include <concepts>
#include <type_traits>
#include <utility>

namespace ptn {

  /* class match_builder start */
  template <typename T, typename... Cases>
  class match_builder {

    /* private tag type used to hide ctor */
    struct ctor_tag {};

    /* private member variables*/
  private:
    T                    value_;
    std::tuple<Cases...> cases_;

    /* public member method */
  public:
    /* perfect forwarding ctor */
    template <class TV, class Tuple>
    requires std::constructible_from<std::tuple<Cases...>, Tuple>
    explicit constexpr match_builder(TV &&v, Tuple &&cs, ctor_tag = {})
        : value_(std::forward<TV>(v)), cases_(std::forward<Tuple>(cs)) {
    }

    /* return itself or a new builder. Only can be called on lvalue */
    template <typename Pattern, typename Handler>
    constexpr auto with(Pattern p, Handler h) & {
      using pair_t   = std::pair<Pattern, Handler>;
      auto new_cases = std::tuple_cat(cases_, std::make_tuple(pair_t{std::move(p), std::move(h)}));
      return match_builder<T, Cases..., pair_t>(value_, std::move(new_cases));
    }

    /* overload for "with", now it supports rvalue. */
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
  };

  /* wrapper */
  template <typename T>
  constexpr auto match(T &&value) noexcept {
    using V = std::decay_t<T>;
    return match_builder<V>(V(std::forward<T>(value)), std::tuple<>());
  }
} // namespace ptn

#endif // PATTERNIA_HPP