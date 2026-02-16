#pragma once

// Public API and implementation for binding patterns (`bind()`).
//
// This file provides factory functions to create patterns that capture and bind
// subject values for later use in pattern matching. It includes both public API
// and internal implementation details, keeping the module self-contained.

#include "ptn/pattern/base/fwd.h"
#include "ptn/pattern/base/binding_base.hpp"
#include "ptn/pattern/base/pattern_base.hpp"
#include "ptn/pattern/structural.hpp"

#include <tuple>
#include <type_traits>
#include <utility>

namespace ptn::pat {

  // Internal Implementation Details

  namespace detail {

    // A pattern that always matches and binds the subject itself.
    // Captures the entire subject as a single-element tuple containing the
    // subject value.
    struct binding_pattern : base::pattern_base<binding_pattern>,
                             base::binding_pattern_base<binding_pattern> {

      // Always matches successfully.
      template <typename Subject>
      constexpr bool match(const Subject &) const noexcept {
        return true;
      }

      // Binds subject itself as a tuple<const Subject&>.
      template <typename Subject>
      constexpr auto bind(const Subject &subject) const {
        return std::forward_as_tuple(subject);
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

      // Binds the subject itself by reference plus sub-pattern bindings.
      template <typename Subject>
      constexpr auto bind(const Subject &subject) const {
        return std::tuple_cat(
            std::forward_as_tuple(subject), subpattern.bind(subject));
      }
    };

    // Detect structural has-pattern
    template <typename T>
    struct is_structural_has : std::false_type {};

    template <auto... Ms>
    struct is_structural_has<pat::detail::has_pattern<Ms...>> : std::true_type {
    };

    template <typename T>
    inline constexpr bool is_structural_has_v =
        is_structural_has<std::decay_t<T>>::value;

    // Forward declaration
    template <typename HasPattern>
    struct structural_bind_pattern;

    // Specialization for has_pattern<Ms...>
    template <auto... Ms>
    struct structural_bind_pattern<pat::detail::has_pattern<Ms...>>
        : base::pattern_base<
              structural_bind_pattern<pat::detail::has_pattern<Ms...>>>,
          base::binding_pattern_base<
              structural_bind_pattern<pat::detail::has_pattern<Ms...>>> {

      using has_type = pat::detail::has_pattern<Ms...>;
      has_type has;

      constexpr explicit structural_bind_pattern(has_type h)
          : has(std::move(h)) {
      }

      template <typename Subject>
      constexpr bool match(const Subject &subject) const noexcept {
        return has.match(subject);
      }

      // Extract member fields for all member-ptr Ms...; ignore wildcard (__).
      template <auto M, typename Subject>
      static constexpr auto bind_one(const Subject &subject) {
        if constexpr (std::is_member_object_pointer_v<decltype(M)>) {
          return std::forward_as_tuple(subject.*M);
        }
        else {
          // nullptr placeholder (_ign) -> ignored
          return std::tuple<>{};
        }
      }

      template <typename Subject>
      constexpr auto bind(const Subject &subject) const {
        return std::tuple_cat(bind_one<Ms>(subject)...);
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

    if constexpr (detail::is_structural_has_v<SP>) {
      return detail::structural_bind_pattern<SP>(
          std::forward<SubPattern>(subpattern));
    }
    else {
      return detail::binding_as_pattern<void, SP>(
          std::forward<SubPattern>(subpattern));
    }
  }

} // namespace ptn::pat

// binding_args Registration

namespace ptn::pat::base {

  // binding_pattern binds 1 value: Subject.
  template <typename Subject>
  struct binding_args<pat::detail::binding_pattern, Subject> {
    using type = std::tuple<const std::remove_reference_t<Subject> &>;
  };

  // binding_as_pattern binds:
  //   (Subject) + binding sequence returned by SubPattern::bind(Subject)
  template <typename Tag, typename SubPattern, typename Subject>
  struct binding_args<
      pat::detail::binding_as_pattern<Tag, SubPattern>,
      Subject> {
    using type = decltype(std::tuple_cat(
        std::tuple<const std::remove_reference_t<Subject> &>{},
        typename binding_args<SubPattern, Subject>::type{}));
  };

  // Structural-binding pattern binds
  template <auto... Ms, typename Subject>
  struct binding_args<
      pat::detail::structural_bind_pattern<pat::detail::has_pattern<Ms...>>,
      Subject> {
    using type =
        decltype(std::declval<const pat::detail::structural_bind_pattern<
                     pat::detail::has_pattern<Ms...>> &>()
                     .bind(std::declval<const Subject &>()));
  };

} // namespace ptn::pat::base
