#pragma once

// Base class and marker type for all Patternia patterns.
//
// This file provides the fundamental CRTP base class that all pattern
// types should inherit from, along with the marker tag type for pattern
// identification.

#include <utility>
#include "pattern_kind.hpp"

namespace ptn::pat::base {

  // Marker tag for Patternia patterns.
  //
  // This empty struct is used as a base class to mark types as patterns.
  // Pattern detection traits check for inheritance from this type.
  struct pattern_tag {};

  // CRTP base class for all pattern types.
  //
  // Template parameter:
  //   Derived: The derived pattern type (CRTP).
  template <typename Derived>
  struct pattern_base : pattern_tag {

    static constexpr pattern_kind kind = pattern_kind::unknown;

    // Forwards the match call to the derived implementation.
    template <typename Subject>
    constexpr bool match(Subject &&subj) const
        noexcept(noexcept(static_cast<const Derived *>(this)->match(
            std::forward<Subject>(subj)))) {
      return static_cast<const Derived *>(this)->match(
          std::forward<Subject>(subj));
    }
  };

} // namespace ptn::pat::base
