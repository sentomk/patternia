#pragma once

#include <type_traits>
#include <tuple>

#include "ptn/patterns/value.hpp"

namespace ptn::dispatch {
  /* categories */
  struct cat_unknown {};
  struct cat_value_eq {};
  struct cat_value_ci {};

  /* default: unknown */
  template <typename P, typename = void>
  struct category_of {
    using type = cat_unknown;
  };

  /* detect value_pattern<Store, std::equal_to<>> */
  template <typename Store, typename Cmp>
  struct is_std_equal_to : std::false_type {};
  template <typename Store>
  struct is_std_equal_to<Store, std::equal_to<>> : std::true_type {};

  /* comparator type for ci_value */
  using ci_cmp_t = ptn::patterns::iequal_ascii;

  /* detect value_pattern<Store, iequal_ascii> */
  template <typename Store, typename Cmp>
  struct is_iequal_ascii : std::is_same<Cmp, ci_cmp_t> {};

  /* category mapping for value patterns */
  template <typename Store, typename Cmp>
  struct category_of<
      ptn::patterns::value_pattern<Store, Cmp>,
      std::enable_if_t<is_std_equal_to<Store, Cmp>::value>> {
    using type = cat_value_eq;
  };
  template <typename Store, typename Cmp>
  struct category_of<
      ptn::patterns::value_pattern<Store, Cmp>,
      std::enable_if_t<is_iequal_ascii<Store, Cmp>::value>> {
    using type = cat_value_ci;
  };

  template <typename P>
  using category_of_t = typename category_of<std::decay_t<P>>::type;

  /* check if all cases belong to Cat. case is pair<Pattern, Handler> */
  template <typename Cat, typename Tuple, std::size_t... I>
  constexpr bool all_cases_are_impl(std::index_sequence<I...>) {
    /* empty tuple should not enable fast path */
    if constexpr (std::tuple_size_v<Tuple> == 0) {
      return false;
    }
    else {
      /* pure fold without side effects */
      return (
          std::is_same_v<
              category_of_t<
                  typename std::tuple_element_t<I, Tuple>::first_type>,
              Cat> &&
          ...);
    }
  }

  template <typename Cat, typename Tuple>
  constexpr bool all_cases_are() {
    return all_cases_are_impl<Cat, Tuple>(
        std::make_index_sequence<std::tuple_size_v<Tuple>>{});
  }
} // namespace ptn::dispatch
