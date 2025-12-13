#pragma once

// Structural (decomposition) pattern: has<...>()
//
// Provides `has<&T::field...>()` to destructure aggregates / records by direct
// non-static data member pointers.
//
//
#include <cstddef>
#include <tuple>
#include <type_traits>

#include "ptn/pattern/base/fwd.h"
#include "ptn/pattern/base/pattern_base.hpp"
#include "ptn/pattern/wildcard.hpp"

namespace ptn::pat {

  namespace detail {

    template <auto M>
    inline constexpr bool is_data_member_ptr_v =
        std::is_member_object_pointer_v<decltype(M)>;

    // C++20 structural wildcard (__)
    template <auto M>
    inline constexpr bool is_structural_wildcard_v =
#if __cpp_nontype_template_args >= 201911L
        std::is_same_v<std::decay_t<decltype(M)>, detail::wildcard_t>;
#else
        false;
#endif

    // C++17 fallback placeholder (rest)
    template <auto M>
    inline constexpr bool is_fallback_placeholder_v =
        std::is_same_v<std::decay_t<decltype(M)>, std::nullptr_t>;

    // Unified notion: structural element
    template <auto M>
    inline constexpr bool is_structural_element_v =
        is_data_member_ptr_v<M> || is_structural_wildcard_v<M> ||
        is_fallback_placeholder_v<M>;

    // has_pattern
    template <auto... Ms>
    struct has_pattern : base::pattern_base<has_pattern<Ms...>> {

      static_assert(
          ((is_data_member_ptr_v<Ms> || is_structural_wildcard_v<Ms> ||
            is_fallback_placeholder_v<Ms>) &&
           ...),
          "has<> only accepts:\n"
          "  - data member pointers (e.g. &T::field)\n"
#if __cpp_nontype_template_args >= 201911L
          "  - '__' as a structural wildcard (C++20)\n"
#endif
          "  - nullptr (or its alias) as a placeholder");

#if __cpp_nontype_template_args < 201911L
      static_assert(
          (!(std::is_same_v<std::decay_t<decltype(Ms)>, detail::wildcard_t>) &&
           ...),
          "has<>: '__' requires C++20 (structural non-type template "
          "parameters).\n"
          "Use nullptr or its alias instead.");
#endif

      // Structural match:
      // If the member expressions are well-formed, the structure matches.
      // Any access failure is a hard compile-time error by design.
      template <typename Subject>
      constexpr bool match(const Subject &) const noexcept {
        return true;
      }

      // IMPORTANT:
      // `has<>` MUST NOT bind anything.
      // Binding is handled exclusively by `bind(...)`.
      template <typename Subject>
      constexpr auto bind(Subject &&) const noexcept {
        return std::tuple<>{};
      }
    };

  } // namespace detail

  // public API
  template <auto... Ms>
  constexpr auto has() {
    return detail::has_pattern<Ms...>{};
  }

} // namespace ptn::pat

namespace ptn::pat::base {

  template <auto... Ms, typename Subject>
  struct binding_args<ptn::pat::detail::has_pattern<Ms...>, Subject> {
    using type = std::tuple<>;
  };
} // namespace ptn::pat::base
