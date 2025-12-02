#pragma once
#include <utility>
#include "pattern_kind.hpp"

namespace ptn::pat::base {

  struct pattern_tag {};

  // CRTP base class for all pattern types.
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
