#pragma once

// Public API and implementation for wildcard patterns (`__`).
//
// This file provides the wildcard pattern that matches any value without
// binding. It includes both public API and internal implementation details,
// keeping the module self-contained.

#include "ptn/pattern/base/fwd.h"
#include "ptn/pattern/base/pattern_base.hpp"

#include <tuple>

namespace ptn::pat {
  namespace detail {
    // Wildcard pattern that matches any value and binds nothing.
    struct wildcard_t : base::pattern_base<wildcard_t> {

      // Always matches successfully.
      template <typename Subject>
      constexpr bool match(Subject &&) const noexcept {
        return true;
      }

      // Binds no values.
      template <typename Subject>
      constexpr auto bind(const Subject &) const {
        return std::tuple<>{};
      }
    };
  } // namespace detail

  // Global wildcard instance.
  inline constexpr ptn::pat::detail::wildcard_t __{};

} // namespace ptn::pat

namespace ptn::pat::base {

  // Wildcard pattern binds no values.
  template <typename Subject>
  struct binding_args<ptn::pat::detail::wildcard_t, Subject> {
    using type = std::tuple<>;
  };
} // namespace ptn::pat::base
