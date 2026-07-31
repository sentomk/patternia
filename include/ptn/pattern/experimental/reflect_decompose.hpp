#pragma once

// =============================================================================
// EXPERIMENTAL: Reflection-based structural decomposition for patternia.
//
// Provides has_refl<^^Type::member...> — a pattern that uses C++26
// reflection (P2996) to destructure structs by reflected member
// references instead of member-object pointers.
//
// Unlike the C++17 has<&T::m...>, this variant:
//   - Works with private/inaccessible members (via access_context)
//   - Provides cleaner error messages via identifier_of
//   - Enables future auto-discovery (has<^^Type> for all members)
//
// Build with:
//   clang++ -std=c++26 -freflection -nostdinc++ \
//     -I<clang-p2996/libcxx/include> -I<patternia/include>
//
// Requires: clang-p2996 fork (bloomberg/clang-p2996 or sentomk/clang-p2996)
// =============================================================================

#if !__has_feature(reflection)
#  error "reflect_decompose.hpp requires C++26 reflection (-freflection)."
#endif

#include <cstddef>
#include <meta>
#include <tuple>
#include <type_traits>
#include <utility>

#include "ptn/pattern/base/binding_base.hpp"
#include "ptn/pattern/base/fwd.h"
#include "ptn/pattern/base/pattern_base.hpp"
#include "ptn/pattern/base/pattern_traits.hpp"

namespace ptn::pat::reflect {

  // -------------------------------------------------------------------------
  // has_refl<info...>: Reflection-powered structural pattern.
  //
  // Accepts std::meta::info values for non-static data members,
  // obtained via the ^^Type::member syntax.
  //
  // Usage:
  //   has_refl<^^Point::x, ^^Point::y>
  //
  // As a non-binding pattern (used in guards):
  //   has_refl<^^Point::x>[_0 == 42] >> handler
  //
  // As a binding pattern (via $):
  //   $(has_refl<^^Point::x, ^^Point::y>) >> [](int x, int y) { ... }
  // -------------------------------------------------------------------------

  // Forward declaration.
  template <typename HasPat, typename Pred>
  struct has_refl_guarded;

  template <std::meta::info... Members>
  struct has_refl_pattern
      : base::pattern_base<has_refl_pattern<Members...>> {

    static_assert(
        (std::meta::is_nonstatic_data_member(Members) && ...),
        "[Patternia.reflect]: all template arguments must be "
        "non-static data member reflections (^^Type::member).");

    // Structural match: always succeeds if the member reflections
    // are valid. The guard predicate handles value-level checks.
    template <typename Subject>
    constexpr bool
    match(const Subject &) const noexcept {
      return true;
    }

    // Extracts the bound member values as a tuple of references.
    // Each member is accessed via splice: subject.[:Member:]
    template <typename Subject>
    constexpr auto
    bind(const Subject &s) const {
      return std::forward_as_tuple(s.[:Members:]...);
    }

    // Creates a guarded version of this pattern.
    template <typename Pred>
    constexpr auto
    operator[](Pred &&pred) const {
      return has_refl_guarded<has_refl_pattern<Members...>,
                              std::decay_t<Pred>>{
          std::forward<Pred>(pred)};
    }
  };

  // -------------------------------------------------------------------------
  // Guarded variant: has_refl<...>[predicate]
  //
  // Matches when the predicate holds over the extracted member tuple.
  // Does not bind — use $(has_refl<...>)[pred] for guarded binding.
  // -------------------------------------------------------------------------

  template <typename HasPat, typename Pred>
  struct has_refl_guarded
      : base::pattern_base<has_refl_guarded<HasPat, Pred>> {
    Pred pred;

    constexpr explicit has_refl_guarded(Pred p)
        : pred(std::move(p)) {
    }

    template <typename Subject>
    constexpr bool
    match(const Subject &s) const {
      auto members = HasPat{}.bind(s);
      return static_cast<bool>(pred(members));
    }

    template <typename Subject>
    constexpr auto
    bind(Subject &&) const noexcept {
      return std::tuple<>{};
    }
  };

  // -------------------------------------------------------------------------
  // Binding variant for $(): wraps has_refl to make it a binding pattern.
  // -------------------------------------------------------------------------

  template <std::meta::info... Members>
  struct has_refl_bind_pattern
      : base::pattern_base<has_refl_bind_pattern<Members...>>,
        base::binding_pattern_base<
            has_refl_bind_pattern<Members...>> {

    template <typename Subject>
    constexpr bool
    match(const Subject &) const noexcept {
      return true;
    }

    template <typename Subject>
    constexpr auto
    bind(const Subject &s) const {
      return std::forward_as_tuple(s.[:Members:]...);
    }
  };

} // namespace ptn::pat::reflect

// ---------------------------------------------------------------------------
// Teach bind_factory ($ operator) about has_refl_bind_pattern.
//
// When the user writes $(reflect::decompose<^^T::m...>()),
// bind_factory::operator() detects the has_refl_bind_pattern and
// returns it directly — avoiding the binding_as_pattern wrapper that
// would try to call alt_index (which doesn't exist on struct patterns).
// ---------------------------------------------------------------------------

namespace ptn::pat::detail {

  // Detects has_refl_bind_pattern (reflection variant).
  template <typename T>
  struct is_reflect_decompose : std::false_type {};

  template <std::meta::info... Ms>
  struct is_reflect_decompose<
      ptn::pat::reflect::has_refl_bind_pattern<Ms...>>
      : std::true_type {};

  template <typename T>
  inline constexpr bool is_reflect_decompose_v =
      is_reflect_decompose<std::decay_t<T>>::value;

} // namespace ptn::pat::detail

namespace ptn::pat::reflect {

  // Public factory object — same name as the C++17 has<>.
  //
  //   reflect::has<^^Point::x, ^^Point::y>      // non-binding check
  //   $(reflect::has<^^Point::x, ^^Point::y>)   // binding

  template <std::meta::info... Members>
  inline constexpr has_refl_pattern<Members...> has{};

  // Binding factory — mirrors $() for the reflection variant.
  //
  //   reflect::bind<^^Point::x, ^^Point::y>
  //   >> [](int x, int y) { return x + y; };

  template <std::meta::info... Members>
  inline constexpr has_refl_bind_pattern<Members...> bind{};

} // namespace ptn::pat::reflect

// ---------------------------------------------------------------------------
// Binding contract declarations.
// ---------------------------------------------------------------------------

namespace ptn::pat::base {

  // has_refl_pattern: non-binding (like has<>).
  template <std::meta::info... Members, typename Subject>
  struct binding_args<ptn::pat::reflect::has_refl_pattern<Members...>,
                      Subject> {
    using type = std::tuple<>;
  };

  // has_refl_guarded: non-binding (like has_guarded_pattern).
  template <typename HasPat, typename Pred, typename Subject>
  struct binding_args<
      ptn::pat::reflect::has_refl_guarded<HasPat, Pred>,
      Subject> {
    using type = std::tuple<>;
  };

  // has_refl_bind_pattern: binds the extracted member values.
  template <std::meta::info... Members, typename Subject>
  struct binding_args<
      ptn::pat::reflect::has_refl_bind_pattern<Members...>,
      Subject> {
    using type = decltype(
        std::declval<
            const ptn::pat::reflect::has_refl_bind_pattern<Members...> &>()
            .bind(std::declval<const Subject &>()));
  };

} // namespace ptn::pat::base
