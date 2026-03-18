#pragma once

// Public API and implementation for literal value patterns (`lit()`,
// `val<>`, and `lit_ci()`).
//
// This file provides factory functions to create patterns that match against
// specific literal values. It includes both public API and internal
// implementation details, keeping the module self-contained.

#include <functional>
#include <string_view>
#include <type_traits>
#include <utility>

#include "ptn/core/common/diagnostics.hpp"
#include "ptn/pattern/base/fwd.h"

namespace ptn::pat {

  // --- Internal Implementation Details ---

  namespace detail {

    // Selects internal storage type for value-like runtime patterns.
    template <typename V>
    using literal_store_t = std::conditional_t<
        std::is_array_v<std::remove_reference_t<V>> ||
            std::is_same_v<std::decay_t<V>, const char *>,
        std::string_view,
        std::decay_t<V>>;

    template <auto V>
    struct static_literal_value_traits {
      using store_t = std::decay_t<decltype(V)>;

      inline static constexpr store_t value = V;
    };

    // Case-insensitive ASCII comparator for string-like values.
    //
    // Performs ASCII-only case folding (`A-Z` -> `a-z`) and compares
    // characters one-by-one.
    struct iequal_ascii {
      static constexpr char tolower_ascii(char c) {
        return (c >= 'A' && c <= 'Z') ? char(c - 'A' + 'a') : c;
      }

      constexpr bool
      operator()(std::string_view a, std::string_view b) const noexcept {
        if (a.size() != b.size()) {
          return false;
        }
        for (std::size_t i = 0; i < a.size(); ++i) {
          if (tolower_ascii(a[i]) != tolower_ascii(b[i])) {
            return false;
          }
        }
        return true;
      }

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
      using store_t = detail::literal_store_t<V>;

      store_t v;

#if defined(__cpp_no_unique_address) && __cpp_no_unique_address >= 201803L
      [[no_unique_address]] Cmp cmp{};
#else
      Cmp cmp{};
#endif

      constexpr literal_pattern(store_t val, Cmp c = {})
          : v(std::move(val)), cmp(std::move(c)) {
      }

      template <typename X>
      constexpr bool match(X const &x) const noexcept(noexcept(cmp(x, v))) {
        return cmp(x, v);
      }

      template <typename X>
      constexpr auto bind(const X & /*subj*/) const {
        return std::tuple<>{};
      }
    };

    // A literal pattern whose matched value is encoded in the type.
    template <auto V, typename Cmp = std::equal_to<>>
    struct static_literal_pattern
        : base::pattern_base<static_literal_pattern<V, Cmp>> {
      using value_traits = static_literal_value_traits<V>;
      using store_t      = typename value_traits::store_t;

      inline static constexpr store_t value = value_traits::value;

#if defined(__cpp_no_unique_address) && __cpp_no_unique_address >= 201803L
      [[no_unique_address]] Cmp cmp{};
#else
      Cmp cmp{};
#endif

      constexpr explicit static_literal_pattern(Cmp c = {})
          : cmp(std::move(c)) {
      }

      template <typename X>
      constexpr bool match(X const &x) const noexcept(
          noexcept(cmp(x, value))) {
        return cmp(x, value);
      }

      template <typename X>
      constexpr auto bind(const X & /*subj*/) const {
        return std::tuple<>{};
      }
    };

  } // namespace detail

  // --- Public API ---

  template <typename V>
  constexpr auto lit(V &&v) {
    using store_t = detail::literal_store_t<V>;
    ptn::core::common::static_assert_literal_store_type<store_t>();

    return detail::literal_pattern<store_t>(store_t(std::forward<V>(v)));
  }

  template <auto V>
  inline constexpr auto val = [] {
    using store_t =
        typename detail::static_literal_value_traits<V>::store_t;
    ptn::core::common::static_assert_literal_store_type<store_t>();
    return detail::static_literal_pattern<V>{};
  }();

  template <typename V>
  constexpr auto lit_ci(V &&v) {
    using store_t = detail::literal_store_t<V>;
    ptn::core::common::static_assert_literal_store_type<store_t>();

    return detail::literal_pattern<store_t, detail::iequal_ascii>(
        store_t(std::forward<V>(v)), detail::iequal_ascii{});
  }

} // namespace ptn::pat

// --- Binding Contract Declaration ---

namespace ptn::pat::base {

  template <typename V, typename Cmp, typename Subject>
  struct binding_args<ptn::pat::detail::literal_pattern<V, Cmp>, Subject> {
    using type = std::tuple<>;
  };

  template <auto V, typename Cmp, typename Subject>
  struct binding_args<ptn::pat::detail::static_literal_pattern<V, Cmp>,
                      Subject> {
    using type = std::tuple<>;
  };

} // namespace ptn::pat::base
