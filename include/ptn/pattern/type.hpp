// Public API and implementation for type-based patterns (`type::is<T>()`).
// Provides variant alternative matching by type, with optional sub-pattern
// matching for binding or structural checks.

#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

#include "ptn/core/common/diagnostics.hpp"
#include "ptn/meta/base/traits.hpp"
#include "ptn/pattern/base/fwd.h"
#include "ptn/pattern/base/pattern_base.hpp"

namespace ptn::pat::type {

  // ------------------------------------------------------------
  // Internal Implementation Details
  // ------------------------------------------------------------

  namespace detail {

    // Matches a specific variant alternative by type (no bindings).
    template <typename T>
    struct type_is_pattern : base::pattern_base<type_is_pattern<T>> {
      using alt_t = meta::remove_cvref_t<T>;

      template <typename Subject>
      constexpr bool match(const Subject &subject) const noexcept {
        core::common::static_assert_variant_alt_unique<alt_t, Subject>();
        return std::holds_alternative<alt_t>(subject);
      }

      template <typename Subject>
      constexpr auto bind(const Subject &) const {
        core::common::static_assert_variant_alt_unique<alt_t, Subject>();
        return std::tuple<>{};
      }
    };

    // Matches a specific alternative and delegates binding to subpattern.
    template <typename T, typename SubPattern>
    struct type_is_with_pattern
        : base::pattern_base<type_is_with_pattern<T, SubPattern>> {
      using alt_t = meta::remove_cvref_t<T>;

      SubPattern subpattern;

      constexpr explicit type_is_with_pattern(SubPattern sub)
          : subpattern(std::move(sub)) {
      }

      template <typename Subject>
      constexpr bool match(const Subject &subject) const {
        core::common::static_assert_variant_alt_unique<alt_t, Subject>();
        if (!std::holds_alternative<alt_t>(subject))
          return false;
        return subpattern.match(std::get<alt_t>(subject));
      }

      template <typename Subject>
      constexpr decltype(auto) bind(const Subject &subject) const {
        core::common::static_assert_variant_alt_unique<alt_t, Subject>();
        return subpattern.bind(std::get<alt_t>(subject));
      }
    };

  } // namespace detail

  // ------------------------------------------------------------
  // Public API
  // ------------------------------------------------------------

  template <typename T>
  constexpr auto is() {
    return detail::type_is_pattern<T>{};
  }

  template <typename T, typename SubPattern>
  constexpr auto is(SubPattern &&subpattern) {
    return detail::type_is_with_pattern<T, std::decay_t<SubPattern>>(
        std::forward<SubPattern>(subpattern));
  }

} // namespace ptn::pat::type

// ------------------------------------------------------------
// Binding Contract Declaration
// ------------------------------------------------------------

namespace ptn::pat::base {

  template <typename T, typename Subject>
  struct binding_args<ptn::pat::type::detail::type_is_pattern<T>, Subject> {
    using type = std::tuple<>;
  };

  template <typename T, typename SubPattern, typename Subject>
  struct binding_args<
      ptn::pat::type::detail::type_is_with_pattern<T, SubPattern>,
      Subject> {
    using alt_t = ptn::meta::remove_cvref_t<T>;
    using type  = typename binding_args<SubPattern, alt_t>::type;
  };

} // namespace ptn::pat::base