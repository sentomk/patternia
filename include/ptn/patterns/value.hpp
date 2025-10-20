#pragma once

#include <functional>
#include <string_view>
#include <type_traits>
#include <utility>
#include "ptn/config.hpp"

namespace ptn::patterns {
  template <typename V>
  /* if is c-style string/array or others */
  using value_store_t = std::conditional_t<
      std::is_array_v<std::remove_reference_t<V>> ||
          std::is_same_v<std::decay_t<V>, const char *>,
      std::string_view,
      std::decay_t<V>>;

  template <typename V, typename Cmp = std::equal_to<>>
  struct value_pattern {
    using store_t = value_store_t<V>;

    store_t v;

#if defined(__cpp_no_unique_address) && __cpp_no_unique_address >= 201803L
    [[no_unique_address]] Cmp cmp{};
#else
    Cmp cmp{};
#endif

    /* allow matching end x to be compared heterogeneously with stored v */
    template <typename X>
    constexpr bool operator()(X const &x) const
        noexcept(noexcept(std::declval<const Cmp &>()(x, v))) {
      return cmp(x, v);
    }
  };

  /* factory: automatic selection of the storage type based on the entry
   * parameter */
  template <typename V>
  constexpr auto value(V &&v) {
    using store_t = value_store_t<V>;
    return value_pattern<store_t>{store_t(std::forward<V>(v))};
  }

  /* case insensitive comparator */
  struct iequal_ascii {
    static constexpr char tolower_ascii(char c) {
      return (c >= 'A' && c <= 'Z') ? char(c - 'A' + 'a') : c;
    }

    /* string_view vsc string_view */
    constexpr bool
    operator()(std::string_view a, std::string_view b) const noexcept {
      if (a.size() != b.size())
        return false;
      for (size_t i = 0; i < a.size(); ++i) {
        if (tolower_ascii(a[i]) != tolower_ascii(b[i]))
          return false;
      }
      return true;
    }

    /* transparent comparison */
#if PTN_USE_CONCEPTS
    template <typename A, typename B>
      requires(
          std::is_convertible_v<A, std::string_view> &&
          std::is_convertible_v<B, std::string_view>)
    constexpr bool operator()(A const &a, B const &b) const noexcept {
      return (*this)(std::string_view(a), std::string_view(b));
    }
#else
    template <
        typename A,
        typename B,
        typename = std::enable_if_t<
            std::is_convertible_v<A, std::string_view> &&
            std::is_convertible_v<B, std::string_view>>>
    constexpr bool operator()(A const &a, B const &b) const noexcept {
      return (*this)(std::string_view(a), std::string_view(b));
    }
#endif
  };

  /* convenience factory: case-insensitive value model */
  template <typename V>
  constexpr auto ci_value(V &&v) {
    using store_t = value_store_t<V>;
    return value_pattern<store_t, iequal_ascii>{
        store_t(std::forward<V>(v)), iequal_ascii{}};
  }
} // namespace ptn::patterns
