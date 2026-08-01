#pragma once

// Base classes and utilities for binding patterns.
//
// This file provides the foundational CRTP base classes that enable
// pattern binding functionality and guard operations. It defines the
// core interfaces that binding patterns must implement.

#include "ptn/pattern/modifiers/fwd.h"
#include <type_traits>
#include <utility>

namespace ptn::pat::base {

  // Hook that lets a binding pattern rewrite its guard predicate
  // before the guarded pattern is built. The default is the
  // identity; structural binding patterns specialize this to
  // resolve PTN_BIND member placeholders against their member
  // list.
  template <typename Derived>
  struct guard_resolver {
    template <typename P>
    static constexpr decltype(auto) apply(P &&pred) {
      return std::forward<P>(pred);
    }
  };

  // CRTP mixin that provides guard operator (`[]`) for binding
  // patterns.
  //
  // This class enables the guard syntax `pattern[predicate]` by
  // providing the operator[] that creates a guarded_pattern.
  //
  // Template parameter:
  //   Derived: The derived pattern type (CRTP).
  template <typename Derived>
  struct guard_operator {
    // Creates a guarded pattern with a predicate.
    //
    // This operator enables the guard syntax: `pattern[predicate]`
    // where the predicate is a callable that takes the bound value
    // and returns a boolean.
    //
    // Template parameter:
    //   Pred: The predicate type (typically a lambda or function).
    // Parameters:
    //   pred: The predicate function to apply to the bound value.
    // Returns: A guarded_pattern that combines the pattern with the
    // predicate.
    template <typename Pred>
    auto operator[](Pred &&pred) const {
      using D       = std::decay_t<Derived>;
      auto resolved = guard_resolver<D>::apply(
          std::forward<Pred>(pred));
      return mod::guarded_pattern<D, decltype(resolved)>{
          static_cast<const D &>(*this), std::move(resolved)};
    }
  };

  // Base class for patterns that can bind values.
  //
  // All binding patterns should inherit from this class to gain
  // guard operator functionality and mark themselves as binding
  // patterns.
  //
  // Template parameter:
  //   Derived: The derived pattern type (CRTP).
  template <typename Derived>
  struct binding_pattern_base : guard_operator<Derived> {
    // Marker indicating this is a binding pattern.
    static constexpr bool is_binding = true;
  };

} // namespace ptn::pat::base
