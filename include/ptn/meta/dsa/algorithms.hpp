#pragma once
/**
 * @file algorithms.hpp
 * @brief Type-level algorithms on type_list and boolean packs:
 *        folds (all/any/none), map<F>, filter<Pred>.
 */

#include <type_traits>
#include "ptn/meta/dsa/type_list.hpp"

namespace ptn::meta {

  //  Fold predicates: all<...>, any<...>, none<...>
  /**
   * @brief Logical AND fold over a pack of boolean template arguments.
   */
  template <bool... Bs>
  struct all : std::bool_constant<(Bs && ...)> {};

  /**
   * @brief Logical OR fold over a pack of boolean template arguments.
   */
  template <bool... Bs>
  struct any : std::bool_constant<(Bs || ...)> {};

  /**
   * @brief Logical NONE fold: true if all Bs are false.
   */
  template <bool... Bs>
  struct none : std::bool_constant<!any<Bs...>::value> {};

  //  map<F, type_list<...>>
  /**
   * @brief Apply a unary metafunction F<T> to every type in a type_list.
   */
  template <template <typename> typename F, typename TL>
  struct map;

  template <template <typename> typename F, typename... Ts>
  struct map<F, type_list<Ts...>> {
    using type = type_list<typename F<Ts>::type...>;
  };

  /**
   * @brief Convenience alias for map.
   */
  template <template <typename> typename F, typename TL>
  using map_t = typename map<F, TL>::type;

  //  filter<Pred, type_list<...>>
  /**
   * @brief Keep only types for which Pred<T>::value == true.
   */
  template <template <typename> typename Pred, typename TL>
  struct filter;

  template <template <typename> typename Pred>
  struct filter<Pred, type_list<>> {
    using type = type_list<>;
  };

  template <template <typename> typename Pred, typename T, typename... Ts>
  struct filter<Pred, type_list<T, Ts...>> {
  private:
    using tail = typename filter<Pred, type_list<Ts...>>::type;

  public:
    using type =
        std::conditional_t<Pred<T>::value, decltype(prepend<T>(tail{})), tail>;
  };

  /**
   * @brief Convenience alias for filter.
   */
  template <template <typename> typename Pred, typename TL>
  using filter_t = typename filter<Pred, TL>::type;

} // namespace ptn::meta
