#pragma once

// Binding contract registration for Patternia patterns.
//
// This file provides the default binding contract that all patterns can
// optionally specialize. Each pattern implementation file should define
// its binding_args specialization to declare what values it binds.
//
// Default: patterns bind no values (empty tuple).

#include <tuple>

namespace ptn::pat::base {

  // Default binding contract: patterns bind nothing.
  template <typename Pattern, typename Subject>
  struct binding_args {
    using type = std::tuple<>;
  };

  // Convenience alias template for binding_args<Pattern, Subject>::type.
  template <typename Pattern, typename Subject>
  using binding_args_t = typename binding_args<Pattern, Subject>::type;

} // namespace ptn::pat::base
