#pragma once

// Compile-time type sequence (type_list) and basic list combinators.

#include <cstddef>

namespace ptn::meta {

  // Simple compile-time container for a pack of types.
  template <typename... Ts>
  struct type_list {
    static constexpr std::size_t size = sizeof...(Ts);
  };

  // Concatenate two type_list sequences.
  template <typename... A, typename... B>
  constexpr auto contact(type_list<A...>, type_list<B...>)
      -> type_list<A..., B...> {
    return {};
  }

  // Append one type to a type_list.
  template <typename T, typename... Ts>
  constexpr auto append(type_list<Ts...>) -> type_list<Ts..., T> {
    return {};
  }

  // Prepend one type to a type_list.
  template <typename T, typename... Ts>
  constexpr auto prepend(type_list<Ts...>) -> type_list<T, Ts...> {
    return {};
  }

  // Retrieve the N-th type from a type_list.
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

  // Convenience alias for nth_type (index-based lookup).
  template <std::size_t N, typename TL>
  using index_t = typename nth_type<N, TL>::type;

} // namespace ptn::meta
