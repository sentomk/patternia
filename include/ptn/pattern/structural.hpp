#pragma once

// Structural (decomposition) pattern: has<...>()
//
// Provides `has<&T::field...>()` to destructure aggregates / records by direct
// non-static data member pointers.
//
//
#include <tuple>
#include <type_traits>

#include "ptn/pattern/base/fwd.h"
#include "ptn/pattern/base/pattern_base.hpp"

namespace ptn::pat {

  namespace detail {

    template <auto M>
    inline constexpr bool is_data_member_ptr_v =
        std::is_member_object_pointer_v<decltype(M)>;

    // has_pattern
    template <auto... Ms>
    struct has_pattern : base::pattern_base<has_pattern<Ms...>> {

      static_assert(
          (is_data_member_ptr_v<Ms> && ...),
          "[Patternia.has]: template parameters must be pointers to "
          "non-static data members.");

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
