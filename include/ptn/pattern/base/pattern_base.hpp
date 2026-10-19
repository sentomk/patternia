#pragma once

// Base class and marker type for all Patternia patterns.
//
// This file provides the fundamental CRTP base class that all
// pattern types should inherit from, along with the marker tag type
// for pattern identification.

#include <cstddef>
#include <type_traits>
#include <utility>
#include "pattern_kind.hpp"

namespace ptn::pat::detail {

  struct no_subpattern;

  template <typename T, typename SubPattern>
  struct type_is_pattern;

  template <std::size_t I, typename SubPattern>
  struct type_alt_pattern;

  template <typename T>
  struct is_factory;

  template <std::size_t I>
  struct alt_factory;

} // namespace ptn::pat::detail

namespace ptn::pat::base {

  // Maps the factory objects behind `is<T>` and `alt<I>` to the
  // canonical pattern types that the dispatch and diagnostic
  // analyses recognise. The factories carry no state, so slicing
  // them to the canonical type is safe.
  template <typename P>
  struct canonical_pattern {
    using type = P;
  };

  template <typename T>
  struct canonical_pattern<ptn::pat::detail::is_factory<T>> {
    using type = ptn::pat::detail::
        type_is_pattern<T, ptn::pat::detail::no_subpattern>;
  };

  template <std::size_t I>
  struct canonical_pattern<ptn::pat::detail::alt_factory<I>> {
    using type = ptn::pat::detail::
        type_alt_pattern<I, ptn::pat::detail::no_subpattern>;
  };

  template <typename P>
  using canonical_pattern_t = typename canonical_pattern<
      std::decay_t<P>>::type;

} // namespace ptn::pat::base

namespace ptn::pat::base {

  // Marker tag for Patternia patterns.
  //
  // This empty struct is used as a base class to mark types as
  // patterns. Pattern detection traits check for inheritance from
  // this type.
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
