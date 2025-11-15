#pragma once
/**
 * @file type_list.hpp
 * @brief Compile-time type sequence (type_list) and basic list combinators.
 *
 * This header defines:
 *   - ptn::meta::type_list<Ts...>   : simple type-level container
 *   - concat                        : concatenate two type_lists
 *   - append                        : append a single type
 *   - prepend                       : prepend a single type
 *   - nth_type<N, TL>               : get the N-th type from a type_list
 */

#include <cstddef>

namespace ptn::meta {

  /**
   * @brief Simple compile-time container for a pack of types.
   *
   * Provides:
   *   - a `size` constant
   *   - a convenient representation for type-level computations
   */
  template <typename... Ts>
  struct type_list {
    static constexpr std::size_t size = sizeof...(Ts);
  };

  /**
   * @brief Concatenate two type_list sequences.
   */
  template <typename... A, typename... B>
  constexpr auto contact(type_list<A...>, type_list<B...>)
      -> type_list<A..., B...> {
    return {};
  }

  /**
   * @brief Append one type to a type_list.
   */
  template <typename T, typename... Ts>
  constexpr auto append(type_list<Ts...>) -> type_list<Ts..., T> {
    return {};
  }

  /**
   * @brief Prepend one type to a type_list.
   */
  template <typename T, typename... Ts>
  constexpr auto prepend(type_list<Ts...>) -> type_list<T, Ts...> {
    return {};
  }

  /**
   * @brief Retrieve the N-th type from a type_list.
   */
  template <std::size_t N, typename TL>
  struct nth_type;

  template <std::size_t N, typename T, typename... Ts>
  struct nth_type<N, type_list<T, Ts...>> {
    using type = typename nth_type<N - 1, type_list<Ts...>>::type;
  };

  template <typename T, typename... Ts>
  struct nth_type<0, type_list<T, Ts...>> {
    using type = T;
  };
  /**
   * @brief Convenience alias for nth_type.
   */
  template <std::size_t N, typename TL>
  using nth_type_t = typename nth_type<N, TL>::type;

} // namespace ptn::meta