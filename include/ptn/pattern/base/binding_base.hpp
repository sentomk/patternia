// Base classes and utilities for binding patterns.
//
// This file provides the foundational CRTP base classes that enable
// pattern binding functionality and guard operations. It defines the
// core interfaces that binding patterns must implement.

#include "ptn/pattern/modifiers/fwd.h"
#include <type_traits>

namespace ptn::pat::base {

  // CRTP mixin that provides guard operator (`[]`) for binding patterns.
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
    // Returns: A guarded_pattern that combines the pattern with the predicate.
    template <typename Pred>
    auto operator[](Pred &&pred) const {
      using D = std::decay_t<Derived>;
      return mod::guarded_pattern<D, std::decay_t<Pred>>{
          static_cast<const D &>(*this), std::forward<Pred>(pred)};
    }
  };

  // Base class for patterns that can bind values.
  //
  // All binding patterns should inherit from this class to gain
  // guard operator functionality and mark themselves as binding patterns.
  //
  // Template parameter:
  //   Derived: The derived pattern type (CRTP).
  template <typename Derived>
  struct binding_pattern_base : guard_operator<Derived> {
    // Marker indicating this is a binding pattern.
    static constexpr bool is_binding = true;
  };

} // namespace ptn::pat::base
