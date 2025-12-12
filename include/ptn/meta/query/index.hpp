#pragma once

// Indexing helpers for type_list: contains<T>, index_of<T>.

#include <type_traits>
#include "ptn/meta/dsa/type_list.hpp"

namespace ptn::meta {

  // contains<T, type_list<...>>

  // Check whether a type_list contains the type T.
  template <typename T, typename TL>
  struct contains;

  template <typename T, typename... Ts>
  struct contains<T, type_list<Ts...>>
      : std::bool_constant<(std::is_same_v<T, Ts> || ...)> {};

  // Convenience value alias for contains.
  template <typename T, typename TL>
  inline constexpr bool contains_v = contains<T, TL>::value;

  // index_of<T, type_list<...>>

  // Compile-time index of T in a type_list (or -1 if not found).
  // Uses a constexpr loop for safe, deterministic behavior.
  template <typename T, typename TL>
  struct index_of : std::integral_constant<int, -1> {};

  template <typename T, typename... Ts>
  struct index_of<T, type_list<Ts...>> {
  private:
    static constexpr int compute() {
      constexpr bool matches[] = {std::is_same_v<T, Ts>...};
      for (int i = 0; i < static_cast<int>(sizeof...(Ts)); ++i)
        if (matches[i])
          return i;
      return -1;
    }

  public:
    static constexpr int value = compute();
  };

  // Convenience value alias for index_of.
  template <typename T, typename TL>
  inline constexpr int index_of_v = index_of<T, TL>::value;

} // namespace ptn::meta
