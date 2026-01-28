#pragma once

// Structural (decomposition) pattern: has<...>()
//
// Provides `has<&T::field...>()` to destructure aggregates / records by direct
// non-static data member pointers.
//
//
#include <cstddef>
#include <tuple>

#include "ptn/pattern/base/fwd.h"
#include "ptn/pattern/base/pattern_base.hpp"
#include "ptn/pattern/base/pattern_traits.hpp"
#include "ptn/core/common/diagnostics.hpp"

namespace ptn::pat {

  // Unified "ignored structural element" placeholder.
  // C++17/20: usable as NTTP (`auto... Ms`) because its type is std::nullptr_t.
  inline constexpr std::nullptr_t _ign = nullptr;

  namespace detail {

    // has_pattern
    template <auto... Ms>
    struct has_pattern : base::pattern_base<has_pattern<Ms...>> {

      constexpr has_pattern() {
        ptn::core::common::static_assert_structural_elements<Ms...>();
      }

      // Structural match:
      // If the member expressions are well-formed, the structure matches.
      // Any access failure is a hard compile-time error by design.
      template <typename Subject>
      constexpr bool match(const Subject &) const noexcept {
        ptn::core::common::static_assert_structural_accessible<Subject, Ms...>();
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
