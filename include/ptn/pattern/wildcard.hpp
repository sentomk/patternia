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
    struct wildcard_t : base::pattern_base<wildcard_t> {

      // Wildcard always matches.
      template <typename Subject>
      constexpr bool match(Subject &&) const noexcept {
        return true;
      }

      // Wildcard binds nothing.
      template <typename Subject>
      constexpr auto bind(const Subject &) const {
        return std::tuple<>{};
      }
    };
  } // namespace detail

  inline constexpr ptn::pat::detail::wildcard_t __{};

} // namespace ptn::pat

namespace ptn::pat::base {

  template <typename Subject>
  struct binding_args<ptn::pat::detail::wildcard_t, Subject> {
    using type = std::tuple<>;
  };
} // namespace ptn::pat::base
