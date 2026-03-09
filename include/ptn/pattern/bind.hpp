#pragma once

// Public API and implementation for binding patterns (`bind()`).
//
// This file provides factory functions to create patterns that
// capture and bind subject values for later use in pattern matching.
// It includes both public API and internal implementation details,
// keeping the module self-contained.

#include "ptn/pattern/base/fwd.h"
#include "ptn/pattern/base/binding_base.hpp"
#include "ptn/pattern/base/pattern_base.hpp"
#include "ptn/pattern/structural.hpp"

#include <tuple>
#include <type_traits>
#include <utility>

namespace ptn::pat {

  // Internal implementation details.

  namespace detail {

    // A pattern that always matches and binds the subject itself.
    // Captures the entire subject as a single-element tuple
    // containing the subject value.
    struct binding_pattern
        : base::pattern_base<binding_pattern>,
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

    // Helper to detect if a type has alt_index method
    template <typename T, typename Subject, typename = void>
    struct has_alt_index : std::false_type {};

    template <typename T, typename Subject>
    struct has_alt_index<
        T,
        Subject,
        std::void_t<decltype(T::template alt_index<Subject>())>>
        : std::true_type {};

    // A pattern that matches using a sub-pattern and binds both the
    // subject and the sub-pattern's bindings. Tag: A tag type for
    // binding identification (typically void). SubPattern: The
    // sub-pattern to use for matching.
    template <typename Tag, typename SubPattern>
    struct binding_as_pattern
        : base::pattern_base<binding_as_pattern<Tag, SubPattern>>,
          base::binding_pattern_base<
              binding_as_pattern<Tag, SubPattern>> {

      SubPattern subpattern;

      constexpr binding_as_pattern(SubPattern sub)
          : subpattern(std::move(sub)) {
      }

      // Uses the sub-pattern for matching.
      template <typename Subject>
      constexpr bool match(const Subject &subject) const noexcept {
        return subpattern.match(subject);
      }

      // Binds the subject itself by reference plus sub-pattern
      // bindings.
      // Special case: if SubPattern is a type_is_pattern or
      // type_alt_pattern with no_subpattern, bind the extracted
      // alternative value instead of the whole variant.
      template <typename Subject>
      constexpr auto bind(const Subject &subject) const {
        // Check if SubPattern has alt_index method (type pattern)
        if constexpr (has_alt_index<SubPattern, Subject>::value) {
          // Check if it has no subpattern (would return empty tuple)
          if constexpr (std::tuple_size_v<decltype(subpattern.bind(
                            subject))>
                        == 0) {
            // Extract and bind the alternative value
            constexpr std::size_t
                idx = SubPattern::template alt_index<Subject>();
            return std::forward_as_tuple(std::get<idx>(subject));
          }
          else {
            // Has subpattern, use normal behavior
            return std::tuple_cat(std::forward_as_tuple(subject),
                                  subpattern.bind(subject));
          }
        }
        else {
          // Not a type pattern, use normal behavior
          return std::tuple_cat(std::forward_as_tuple(subject),
                                subpattern.bind(subject));
        }
      }
    };

    // Detects `has<...>` structural patterns.
    template <typename T>
    struct is_structural_has : std::false_type {};

    template <auto... Ms>
    struct is_structural_has<pat::detail::has_pattern<Ms...>>
        : std::true_type {};

    template <typename T>
    inline constexpr bool is_structural_has_v = is_structural_has<
        std::decay_t<T>>::value;

    // Forward declaration.
    template <typename HasPattern>
    struct structural_bind_pattern;

    // Specialization for `has_pattern<Ms...>`.
    template <auto... Ms>
    struct structural_bind_pattern<pat::detail::has_pattern<Ms...>>
        : base::pattern_base<structural_bind_pattern<
              pat::detail::has_pattern<Ms...>>>,
          base::binding_pattern_base<structural_bind_pattern<
              pat::detail::has_pattern<Ms...>>> {

      using has_type = pat::detail::has_pattern<Ms...>;
      has_type has;

      constexpr explicit structural_bind_pattern(has_type h)
          : has(std::move(h)) {
      }

      template <typename Subject>
      constexpr bool match(const Subject &subject) const noexcept {
        return has.match(subject);
      }

      // Extract member fields for all member-ptr Ms...; ignore
      // wildcard (__).
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

  // Public API.
  //
  // Note: The bind(v) / bind(v, subpattern) forms are not provided.
  // In C++, v would be treated as a variable name in the DSL
  // context, which would result in a "v undeclared" compilation
  // error.

  // bind() - Captures the current subject itself.
  constexpr auto bind() {
    // Use void as Tag for placeholder purposes.
    return detail::binding_pattern{};
  }

  // bind(subpattern) - First matches with subpattern, then captures
  // subject.
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

  // bind_factory - Callable object that acts as both a binding
  // pattern and a factory for creating binding patterns with
  // subpatterns.
  //
  // This allows $ to work in multiple ways:
  //   $ >> handler                    // direct binding
  //   $[_0 > 0] >> handler            // binding with guard
  //   $(has<&T::x>()) >> handler      // binding with subpattern
  //   $(is<T>()) >> handler           // binding variant type
  struct bind_factory : base::pattern_base<bind_factory>,
                        base::binding_pattern_base<bind_factory> {

    // Act as a binding pattern directly (for $ >> handler)
    template <typename Subject>
    constexpr bool match(const Subject &) const noexcept {
      return true;
    }

    template <typename Subject>
    constexpr auto bind(const Subject &subject) const {
      return std::forward_as_tuple(subject);
    }

    // operator()() - Return binding_pattern for $() form
    constexpr auto operator()() const {
      return detail::binding_pattern{};
    }

    // operator()(subpattern) - Return appropriate binding pattern
    template <typename SubPattern>
    constexpr auto operator()(SubPattern &&subpattern) const {
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
  };

  // $ - Shorthand for bind() that can also be called with
  // subpatterns.
  //
  // Usage:
  //   $ >> handler                    // bind whole subject
  //   $[_0 > 0] >> handler            // bind with guard
  //   $(has<&T::x, &T::y>()) >> handler  // bind structural members
  //   $(is<T>()) >> handler           // bind variant alternative
  inline constexpr bind_factory $;

  // ds<&T::m...>() - Destructure and bind member fields.
  //
  // Shorthand for bind(has<&T::m...>()). Matches structurally and
  // binds the specified member fields for use in the handler:
  //   ds<&Point::x, &Point::y>() >> [](int x, int y) { ... }
  //   ds<&Point::x, &Point::y>()[_0 > 0] >> [](int x, int y) { ... }
  template <auto... Ms>
  constexpr auto ds() {
    return detail::structural_bind_pattern<
        detail::has_pattern<Ms...>>(detail::has_pattern<Ms...>{});
  }

} // namespace ptn::pat

// Binding-args registration.

namespace ptn::pat::base {

  // `binding_pattern` binds one value: Subject.
  template <typename Subject>
  struct binding_args<pat::detail::binding_pattern, Subject> {
    using type = std::tuple<
        const std::remove_reference_t<Subject> &>;
  };

  // `bind_factory` binds one value: Subject (same as
  // binding_pattern).
  template <typename Subject>
  struct binding_args<pat::bind_factory, Subject> {
    using type = std::tuple<
        const std::remove_reference_t<Subject> &>;
  };

  // `binding_as_pattern` binds:
  //   Special case: if SubPattern is a type pattern with no
  //   subpattern, bind the extracted alternative value.
  //   Otherwise: (Subject) + binding sequence returned by
  //   SubPattern::bind(Subject)
  template <typename Tag, typename SubPattern, typename Subject>
  struct binding_args<
      pat::detail::binding_as_pattern<Tag, SubPattern>,
      Subject> {
  private:
    // Check if SubPattern has alt_index (is a type pattern)
    static constexpr bool
        is_type_pattern = pat::detail::has_alt_index<SubPattern,
                                                     Subject>::value;

    // Check if SubPattern binds nothing (empty tuple)
    static constexpr bool
        has_no_subpattern = std::tuple_size_v<
                                typename binding_args<SubPattern,
                                                      Subject>::type>
                            == 0;

  public:
    using type = std::conditional_t<
        is_type_pattern && has_no_subpattern,
        // Type pattern with no subpattern: bind extracted value
        std::tuple<const std::variant_alternative_t<
            SubPattern::template alt_index<Subject>(),
            std::remove_reference_t<Subject>> &>,
        // Otherwise: bind Subject + SubPattern bindings
        decltype(std::tuple_cat(
            std::declval<std::tuple<
                const std::remove_reference_t<Subject> &>>(),
            std::declval<typename binding_args<SubPattern,
                                               Subject>::type>()))>;
  };

  // Structural-binding pattern binds.
  template <auto... Ms, typename Subject>
  struct binding_args<pat::detail::structural_bind_pattern<
                          pat::detail::has_pattern<Ms...>>,
                      Subject> {
    using type =
        decltype(std::declval<
                     const pat::detail::structural_bind_pattern<
                         pat::detail::has_pattern<Ms...>> &>()
                     .bind(std::declval<const Subject &>()));
  };

} // namespace ptn::pat::base
