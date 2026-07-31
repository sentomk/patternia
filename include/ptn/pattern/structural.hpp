#pragma once

// Structural (decomposition) pattern: `has<...>`.
//
// Provides `has<&T::field...>` and, when reflection is enabled,
// `has<^^T::field...>` to destructure aggregates and records.

#include <cstddef>
#include <tuple>

#if defined(__has_feature)
#if __has_feature(reflection)
#include <meta>
#define PTN_DETAIL_HAS_REFLECTION 1
#endif
#endif

#include "ptn/pattern/base/fwd.h"
#include "ptn/pattern/base/pattern_base.hpp"
#include "ptn/pattern/base/pattern_traits.hpp"
#include "ptn/core/common/diagnostics.hpp"

namespace ptn::pat {

  // Unified "ignored structural element" placeholder.
  // C++17/20: usable as NTTP (`auto... Ms`) because its type is
  // std::nullptr_t.
  inline constexpr std::nullptr_t _ign = nullptr;

  namespace detail {

    template <auto M>
    constexpr bool is_valid_structural_element() {
#if defined(PTN_DETAIL_HAS_REFLECTION)
      if constexpr (std::is_same_v<std::decay_t<decltype(M)>,
                                   std::meta::info>) {
        return std::meta::is_nonstatic_data_member(M);
      }
      else
#endif
      {
        return traits::is_structural_element_v<M>;
      }
    }

#if defined(PTN_DETAIL_HAS_REFLECTION)
    template <typename Subject, auto M>
    constexpr bool is_structural_element_accessible() {
      if constexpr (std::is_same_v<std::decay_t<decltype(M)>,
                                   std::meta::info>) {
        return requires(const Subject &subject) { subject.[:M:]; };
      }
      else if constexpr (std::is_null_pointer_v<decltype(M)>) {
        return true;
      }
      else {
        return requires(const Subject &subject) { subject.*M; };
      }
    }
#endif

    // Extracts the value of a single member pointer or reflection
    // from a subject.
    template <auto M, typename Subject>
    constexpr decltype(auto)
    extract_member(const Subject &s) noexcept {
      if constexpr (std::is_null_pointer_v<decltype(M)>) {
        // _ign: ignored slot, never extracted.
        return;
      }
#if defined(PTN_DETAIL_HAS_REFLECTION)
      else if constexpr (std::is_same_v<std::decay_t<decltype(M)>,
                                        std::meta::info>) {
        return (s.[:M:]);
      }
#endif
      else {
        return (s.*M);
      }
    }

    // Extracts all non-ignored members into a tuple of references.
    template <typename Subject, auto... Ms>
    constexpr auto extract_members(const Subject &s) noexcept {
      return std::tuple_cat([&]() {
        if constexpr (std::is_null_pointer_v<decltype(Ms)>) {
          return std::tuple<>{};
        }
#if defined(PTN_DETAIL_HAS_REFLECTION)
        else if constexpr (std::is_same_v<std::decay_t<decltype(Ms)>,
                                          std::meta::info>) {
          return std::forward_as_tuple(s.[:Ms:]);
        }
#endif
        else {
          return std::forward_as_tuple(s.*Ms);
        }
      }()...);
    }

    // Implements `has<...>` as a non-binding structural matcher.
    template <auto... Ms>
    struct has_pattern : base::pattern_base<has_pattern<Ms...>> {

      constexpr has_pattern() {
        static_assert(
            (is_valid_structural_element<Ms>() && ...),
            "[Patternia.has]: only non-static data member pointers, "
            "non-static data member reflections, or nullptr/_ign "
            "allowed.");
      }

      // Performs structural matching.
      // If the member expressions are well-formed, the structure
      // matches. Any access failure is a hard compile-time error by
      // design.
      template <typename Subject>
      constexpr bool match(const Subject &) const noexcept {
#if defined(PTN_DETAIL_HAS_REFLECTION)
        static_assert(
            (is_structural_element_accessible<Subject, Ms>() && ...),
            "[Patternia.has]: Subject lacks a requested member.");
#else
        ptn::core::common::
            static_assert_structural_accessible<Subject, Ms...>();
#endif
        return true;
      }

      // IMPORTANT:
      // `has<>` MUST NOT bind anything.
      // Binding is handled exclusively by `$` and `$(...)`.
      template <typename Subject>
      constexpr auto bind(Subject &&) const noexcept {
        return std::tuple<>{};
      }

      // Creates a guarded structural pattern.
      //
      // The guard predicate receives the extracted members as a
      // tuple for evaluation, but the resulting pattern still does
      // not bind any values to the handler.
      //
      // Usage:
      //   has<&Pkt::type, &Pkt::len>[_0 == 0x01 && arg<1> == 0]
      //       >> [] { handle_ping(); }
      template <typename Pred>
      constexpr auto operator[](Pred &&pred) const;
    };

    // Structural pattern with guard: matches when the guard
    // predicate holds over the extracted members, but does not bind
    // any values.
    //
    // Template parameters:
    //   HasPat: The underlying has_pattern<Ms...> type.
    //   Pred:   The guard predicate type.
    template <typename HasPat, typename Pred>
    struct has_guarded_pattern
        : base::pattern_base<has_guarded_pattern<HasPat, Pred>> {
      Pred pred;

      template <typename P>
      constexpr explicit has_guarded_pattern(P &&p)
          : pred(std::forward<P>(p)) {
      }

      template <typename Subject>
      constexpr bool match(const Subject &s) const {
        // Delegate structural accessibility check to has_pattern.
        HasPat{}.match(s);
        auto members = extract_members_from_has(HasPat{}, s);
        return static_cast<bool>(pred(members));
      }

      template <typename Subject>
      constexpr auto bind(Subject &&) const noexcept {
        return std::tuple<>{};
      }
    };

    // Extracts members from a has_pattern's member pointer list.
    template <auto... Ms, typename Subject>
    constexpr auto
    extract_members_from_has(has_pattern<Ms...>,
                             const Subject &s) noexcept {
      return extract_members<Subject, Ms...>(s);
    }

    // Deferred definition of has_pattern::operator[].
    template <auto... Ms>
    template <typename Pred>
    constexpr auto
    has_pattern<Ms...>::operator[](Pred &&pred) const {
      return has_guarded_pattern<has_pattern<Ms...>,
                                 std::decay_t<Pred>>{
          std::forward<Pred>(pred)};
    }

  } // namespace detail

  // Public API.
  template <auto... Ms>
  inline constexpr detail::has_pattern<Ms...> has{};

} // namespace ptn::pat

namespace ptn::pat::base {

  template <auto... Ms, typename Subject>
  struct binding_args<ptn::pat::detail::has_pattern<Ms...>,
                      Subject> {
    using type = std::tuple<>;
  };

  template <typename HasPat, typename Pred, typename Subject>
  struct binding_args<
      ptn::pat::detail::has_guarded_pattern<HasPat, Pred>,
      Subject> {
    using type = std::tuple<>;
  };
} // namespace ptn::pat::base

#if defined(PTN_DETAIL_HAS_REFLECTION)
#undef PTN_DETAIL_HAS_REFLECTION
#endif
