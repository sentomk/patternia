#pragma once

#include <functional>
#include <string_view>
#include <type_traits>
#include <utility>
#include "ptn/pattern/pattern_base.hpp" // use pattern_base
#include "ptn/config.hpp"

/**
 * @file literal_pattern.hpp
 * @brief Value-based patterns (`lit()` and `lit_ci()`).
 *
 * This module provides the canonical equality-comparison pattern for Patternia.
 * A `literal_pattern<T>` stores a reference value and matches subjects via an
 * equality comparator (default: `std::equal_to<>`).
 *
 *
 * Part of the Pattern Layer (namespace ptn::pattern::value)
 */

namespace ptn::pattern::value {

  /**
   * @brief Selects the internal storage type for value-like patterns.
   *
   * C-style strings and string literals are stored as `std::string_view`.
   * All other types use `std::decay_t<V>`.
   *
   * @tparam V Input value type.
   */
  template <typename V>
  /* if is c-style string/array or others */
  using literal_store_t = std::conditional_t<
      std::is_array_v<std::remove_reference_t<V>> ||
          std::is_same_v<std::decay_t<V>, const char *>,
      std::string_view,
      std::decay_t<V>>;

  /**
   * @brief A pattern that matches a subject by comparing it with a stored
   * value.
   *
   * @tparam V   The stored value type (already decayed/string-view adapted).
   * @tparam Cmp Comparator type (`Cmp(a, b)` → bool). Default:
   * `std::equal_to<>`.
   *
   * This pattern inherits from pattern_base, therefore supporting:
   * - `match(subject)` via CRTP forwarding
   * - `bind(subject)` defaulting to returning the subject unchanged
   */
  template <typename V, typename Cmp = std::equal_to<>>
  struct literal_pattern : pattern::pattern_base<literal_pattern<V, Cmp>> {
    using store_t = literal_store_t<V>;

    /** @brief Stored value for matching. */
    store_t v;

#if defined(__cpp_no_unique_address) && __cpp_no_unique_address >= 201803L
    /** @brief Comparator for equality-like matching (no_unique_address if
     * available). */
    [[no_unique_address]] Cmp cmp{};
#else
    Cmp cmp{};
#endif

    /**
     * @brief Constructs a literal_pattern with a stored value and comparator.
     * @param val Value to store.
     * @param c   Comparator instance (default-constructed by default).
     */
    constexpr literal_pattern(store_t val, Cmp c = {})
        : v(std::move(val)), cmp(std::move(c)) {
    }

    /**
     * @brief Performs the actual comparison.
     *
     * @tparam X Subject type.
     * @param x  The subject value.
     * @return `true` if `cmp(x, v)` returns true.
     */
    template <typename X>
    constexpr bool match(X const &x) const noexcept(noexcept(cmp(x, v))) {
      return cmp(x, v);
    }

    template <typename X>
    constexpr bool operator()(X const &x) const
        noexcept(noexcept(this->match(x))) {
      return this->match(x);
    }

    /**
     * @brief Binding result for value-patterns.
     *
     * Returns the stored value `v`, enabling value extraction:
     */
    template <typename X>
    constexpr const store_t &bind(X const &) const noexcept {
      return v;
    }
  };

  /**
   * @brief Factory function that constructs a value-pattern.
   *
   * Automatically adapts C-style strings into `std::string_view`, otherwise
   * stores values as `std::decay_t<V>`.
   */
  template <typename V>
  constexpr auto lit(V &&v) {
    using store_t = literal_store_t<V>;
    return literal_pattern<store_t>(store_t(std::forward<V>(v)));
  }

  /**
   * @brief Case-insensitive ASCII comparator for string-like values.
   *
   * Performs ASCII-only case folding (`A-Z` → `a-z`) and compares characters
   * one-by-one.
   *
   * Supports heterogeneous comparisons via transparent operator() overloads
   * accepting any type convertible to `std::string_view`.
   */
  struct iequal_ascii : pattern::detail::pattern_tag {

    /** @brief ASCII-only lowercase conversion. */
    static constexpr char tolower_ascii(char c) {
      return (c >= 'A' && c <= 'Z') ? char(c - 'A' + 'a') : c;
    }

    /** @brief Case-insensitive comparison of two string_view values. */
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

#if PTN_USE_CONCEPTS

    /**
     * @brief Transparent heterogeneous case-insensitive comparison (C++20).
     *
     * Accepts any A/B convertible to `std::string_view`.
     */
    template <typename A, typename B>
      requires(
          std::is_convertible_v<A, std::string_view> &&
          std::is_convertible_v<B, std::string_view>)
    constexpr bool operator()(A const &a, B const &b) const noexcept {
      return (*this)(std::string_view(a), std::string_view(b));
    }
#else

    /**
     * @brief Transparent heterogeneous case-insensitive comparison (C++17).
     */
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

  /**
   * @brief Case-insensitive value-pattern factory.
   *
   * Used for matching command strings, tokens, or user input without
   * case sensitivity.
   *
   * @tparam V Input value type (deduced).
   * @param v  Value to store.
   * @return A `literal_pattern` using `iequal_ascii` comparator.
   */
  template <typename V>
  constexpr auto lit_ci(V &&v) {
    using store_t = literal_store_t<V>;
    return literal_pattern<store_t, iequal_ascii>(
        store_t(std::forward<V>(v)), iequal_ascii{});
  }
} // namespace ptn::pattern::value
