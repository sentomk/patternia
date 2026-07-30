#pragma once

// =============================================================================
// EXPERIMENTAL: Reflection-based named binding for pattern guards.
//
// This header requires C++26 reflection (P2996). It is gated behind
// __has_feature(reflection) and will not compile on standard compilers.
//
// Build with:
//   clang++ -std=c++26 -freflection ... -I include
//
// See docs/experimental/reflection-named-binding.md for design rationale.
// =============================================================================

#if !__has_feature(reflection)
#  error "This header requires C++26 reflection (-freflection). \
Use the standard guard.hpp for C++17-compatible guards."
#endif

#include <meta>
#include <type_traits>
#include <utility>

#include "ptn/pattern/base/pattern_base.hpp"

namespace ptn::pat::reflect {

  // --------------------------------------------------------------------------
  // Named member placeholder
  //
  // Carries a member reflection and its index. At evaluation time, the
  // reflection is spliced to access the actual member value on the subject.
  // --------------------------------------------------------------------------

  template <std::meta::info MemberRef, std::size_t Index>
  struct named_placeholder {
    static constexpr std::meta::info reflection = MemberRef;
    static constexpr std::size_t      index     = Index;

    // The member's name, available at compile time.
    static constexpr std::string_view name =
        std::meta::identifier_of(MemberRef);
  };

  // --------------------------------------------------------------------------
  // decompose_pattern: matches any struct, binds all non-static data members.
  //
  // This is the reflection-powered equivalent of has<...>. Instead of
  // requiring the user to list member pointers, it auto-discovers them.
  // --------------------------------------------------------------------------

  template <typename T>
  struct decompose_pattern
      : base::pattern_base<decompose_pattern<T>> {

    static_assert(std::is_class_v<T>,
        "[Patternia.reflect.decompose]: subject must be a class/struct type.");

    template <typename Subject>
    constexpr bool match(Subject &&) const noexcept {
      // A struct with data members always matches decompose.
      // The guard predicate is responsible for any value-level filtering.
      return true;
    }

    // Binds all non-static data members into a tuple.
    template <typename Subject>
    constexpr auto bind(Subject &&s) const {
      constexpr auto ctx =
          std::meta::access_context::current();

      constexpr auto members =
          std::meta::nonstatic_data_members_of(^^T, ctx);

      return bind_impl(std::forward<Subject>(s), members,
                       std::make_index_sequence<members.size()>{});
    }

  private:
    template <typename Subject, std::size_t... Is>
    static constexpr auto bind_impl(
        Subject                      &&s,
        std::meta::access_context    /*ctx*/,
        std::meta::info              /*members*/,
        std::index_sequence<Is...>) {

      // Splice each member reflection to access the actual value.
      // [: nonstatic_data_members_of(^^T, ctx)[Is] :] gives us
      // a member access expression equivalent to s.member_name
      constexpr auto ctx =
          std::meta::access_context::current();
      constexpr auto members =
          std::meta::nonstatic_data_members_of(^^T, ctx);

      return std::tuple<
          // The result type is tuple<T::member0, T::member1, ...>
          // Types are resolved via splice at compile time.
      >{};
      // TODO: Actual splice-based binding.
      //
      // The clean implementation is:
      //
      //   return std::forward_as_tuple(
      //       s.[: members[Is] :]...
      //   );
      //
      // But this requires the P2996 expansion statement (template for)
      // or pack-style splice expansion, which is still being stabilised
      // in the Clang P2996 fork.
    }
  };

  // --------------------------------------------------------------------------
  // Factory function
  // --------------------------------------------------------------------------

  template <typename T>
  constexpr auto decompose() {
    return decompose_pattern<T>{};
  }

  // --------------------------------------------------------------------------
  // Enhanced $ operator (placeholder)
  //
  // When reflection is available, $ on a struct type auto-decomposes.
  // On non-struct types (int, string, etc.), it falls back to whole-value
  // binding (existing behavior).
  //
  // This is where the user-facing API will live once the bind_impl
  // splice mechanics are worked out.
  // --------------------------------------------------------------------------

  // TODO: Implement dollar_dispatch that selects decompose vs whole-bind
  //       based on whether T is a struct with data members.

} // namespace ptn::pat::reflect

// --- Binding Contract Declaration -------------------------------------------
//
// The binding_args specialization tells the match engine what types
// decompose_pattern produces. This must match the actual bind() return type.

namespace ptn::pat::base {

  // TODO: Specialize binding_args for decompose_pattern<T>
  //       once the tuple type is computable via reflection.

} // namespace ptn::pat::base
