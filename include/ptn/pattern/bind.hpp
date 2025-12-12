#pragma once

// Public API and implementation for binding patterns (`bind()`).
//
// This file provides factory functions to create patterns that capture and bind
// subject values for later use in pattern matching. It includes both public API
// and internal implementation details, keeping the module self-contained.

#include "ptn/pattern/base/fwd.h"
#include "ptn/pattern/base/binding_base.hpp"
#include "ptn/pattern/base/pattern_base.hpp"

#include <tuple>
#include <type_traits>
#include <utility>

namespace ptn::pat {

  // Internal Implementation Details

  namespace detail {

    // A pattern that always matches and binds the subject itself.
    // Captures the entire subject as a single-element tuple containing the subject value.
    struct binding_pattern : base::pattern_base<binding_pattern>,
                             base::binding_pattern_base<binding_pattern> {

      // Always matches successfully.
      template <typename Subject>
      constexpr bool match(const Subject &) const noexcept {
        return true;
      }

      // Binds subject itself as a tuple<Subject>.
      template <typename Subject>
      constexpr auto bind(const Subject &subject) const {
        return std::tuple<Subject>(subject);
      }
    };

    // A pattern that matches using a sub-pattern and binds both the subject
    // and the sub-pattern's bindings.
    // Tag: A tag type for binding identification (typically void).
    // SubPattern: The sub-pattern to use for matching.
    template <typename Tag, typename SubPattern>
    struct binding_as_pattern
        : base::pattern_base<binding_as_pattern<Tag, SubPattern>>,
          base::binding_pattern_base<binding_as_pattern<Tag, SubPattern>> {

      SubPattern subpattern;

      constexpr binding_as_pattern(SubPattern sub)
          : subpattern(std::move(sub)) {
      }

      // Uses the sub-pattern for matching.
      template <typename Subject>
      constexpr bool match(const Subject &subject) const noexcept {
        return subpattern.match(subject);
      }

      // Binds the subject itself plus the sub-pattern's binding values.
      template <typename Subject>
      constexpr auto bind(const Subject &subject) const {
        return std::tuple_cat(
            std::tuple<Subject>(subject), subpattern.bind(subject));
      }
    };

  } // namespace detail

  // Public API
  //
  // Note: The bind(v) / bind(v, subpattern) forms are not provided.
  // In C++, v would be treated as a variable name in the DSL context,
  // which would result in a "v undeclared" compilation error.

  // bind() - Captures the current subject itself.
  constexpr auto bind() {
    // Use void as Tag for placeholder purposes.
    return detail::binding_pattern{};
  }

  // bind(subpattern) - First matches with subpattern, then captures subject.
  template <typename SubPattern>
  constexpr auto bind(SubPattern &&subpattern) {
    using SP = std::decay_t<SubPattern>;
    return detail::binding_as_pattern<void, SP>(
        std::forward<SubPattern>(subpattern));
  }

} // namespace ptn::pat

// binding_args Registration

namespace ptn::pat::base {

  // binding_pattern binds 1 value: Subject.
  template <typename Subject>
  struct binding_args<pat::detail::binding_pattern, Subject> {
    using type = std::tuple<Subject>;
  };

  // binding_as_pattern binds:
  //   (Subject) + binding sequence returned by SubPattern::bind(Subject)
  template <typename Tag, typename SubPattern, typename Subject>
  struct binding_args<
      pat::detail::binding_as_pattern<Tag, SubPattern>,
      Subject> {
    using type = decltype(std::tuple_cat(
        std::tuple<Subject>{},
        typename binding_args<SubPattern, Subject>::type{}));
  };

} // namespace ptn::pat::base
