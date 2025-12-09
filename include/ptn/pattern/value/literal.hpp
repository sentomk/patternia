#pragma once

// Public API and implementation for literal value patterns (`lit()` and
// `lit_ci()`).
//
// This file provides factory functions to create patterns that match against
// specific literal values. It includes both public API and internal
// implementation details, keeping the module self-contained.

#include <functional>
#include <string_view>
#include <type_traits>
#include <utility>

#include "ptn/pattern/base/fwd.h"

namespace ptn::pat::value {

  // --- Internal Implementation Details ---

  namespace detail {

    // Selects internal storage type for value-like patterns.
    //
    // C-style strings and string literals are stored as `std::string_view`.
    // All other types use `std::decay_t<V>`.
    //
    // Template parameter:
    //   V: Input value type.
    template <typename V>
    using literal_store_t = std::conditional_t<
        std::is_array_v<std::remove_reference_t<V>> ||
            std::is_same_v<std::decay_t<V>, const char *>,
        std::string_view,
        std::decay_t<V>>;

    // Case-insensitive ASCII comparator for string-like values.
    //
    // Performs ASCII-only case folding (`A-Z` → `a-z`) and compares characters
    // one-by-one.
    //
    struct iequal_ascii {
      // ASCII-only lowercase conversion.
      static constexpr char tolower_ascii(char c) {
        return (c >= 'A' && c <= 'Z') ? char(c - 'A' + 'a') : c;
      }

      // Case-insensitive comparison of two string_view values.
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

      // Transparent heterogeneous case-insensitive comparison (C++17).
      template <
          typename A,
          typename B,
          typename = std::enable_if_t<
              std::is_convertible_v<A, std::string_view> &&
              std::is_convertible_v<B, std::string_view>>>
      constexpr bool operator()(A const &a, B const &b) const noexcept {
        return (*this)(std::string_view(a), std::string_view(b));
      }
    };

    // A pattern that matches a subject by comparing it with a stored value.
    template <typename V, typename Cmp = std::equal_to<>>
    struct literal_pattern : base::pattern_base<literal_pattern<V, Cmp>> {
      using store_t = literal_store_t<V>;

      // Stored value for matching.
      store_t v;

#if defined(__cpp_no_unique_address) && __cpp_no_unique_address >= 201803L
      [[no_unique_address]] Cmp cmp{};
#else
      Cmp cmp{};
#endif

      // Constructs a literal_pattern with a stored value and comparator.
      constexpr literal_pattern(store_t val, Cmp c = {})
          : v(std::move(val)), cmp(std::move(c)) {
      }

      // Performs the actual comparison.
      template <typename X>
      constexpr bool match(X const &x) const noexcept(noexcept(cmp(x, v))) {
        return cmp(x, v);
      }

      // Binding result for literal patterns.
      //
      // Literal patterns do not bind any values, returning an empty tuple.
      template <typename X>
      constexpr auto bind(const X & /*subj*/) const {
        return std::tuple<>{};
      }
    };

  } // namespace detail

  // --- Public API ---

  // Factory for literal_pattern.
  template <typename V>
  constexpr auto lit(V &&v) {
    using store_t = detail::literal_store_t<V>;

    static_assert(
        !std::is_void_v<store_t>,
        "[Patternia.lit]: Literal value cannot be of type void.");
    static_assert(
        !std::is_reference_v<store_t>,
        "[Patternia.lit]: Literal value must be a value type (non-reference).");
    static_assert(
        std::is_move_constructible_v<store_t>,
        "[Patternia.lit]: Literal value must be move-constructible.");
    static_assert(
        std::is_constructible_v<
            bool,
            decltype(std::declval<const store_t &>() == std::declval<const store_t &>())>,
        "[Patternia.lit]: Literal value type must support operator==.");

    return detail::literal_pattern<store_t>(store_t(std::forward<V>(v)));
  }

  // Case-insensitive value-pattern factory.
  template <typename V>
  constexpr auto lit_ci(V &&v) {
    using store_t = detail::literal_store_t<V>;
    return detail::literal_pattern<store_t, detail::iequal_ascii>(
        store_t(std::forward<V>(v)), detail::iequal_ascii{});
  }

} // namespace ptn::pat::value

// --- Binding Contract Declaration ---

namespace ptn::pat::base {

  // Specialization of the binding contract for
  // `detail::literal_pattern`.
  //
  // This specialization declares that a literal_pattern binds no arguments,
  // which corresponds to an empty `std::tuple<>`. This declaration must be
  // consistent with the return type of `detail::literal_pattern::bind()`.
  template <typename V, typename Cmp, typename Subject>
  struct binding_args<
      ptn::pat::value::detail::literal_pattern<V, Cmp>,
      Subject> {
    using type = std::tuple<>;
  };

} // namespace ptn::pat::base
