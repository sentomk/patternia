#pragma once

#include <cstddef>
#include <type_traits>

namespace ptn::pat::mod {

  // Internal placeholder node for the Nth bound value.
  template <std::size_t I>
  struct arg_t {
    static constexpr std::size_t index = I;
  };

  // Member-anchored placeholder for structural guards.
  //
  // A PTN_BIND name expands to member_t<&Type::member>. Inside a
  // guard attached to has<...>, the structural pattern resolves
  // the member pointer to the position of that member in its
  // member list at compile time, so guard names follow members,
  // not the order of pointers in has<...>.
  template <auto M>
  struct member_t {
    static constexpr auto member = M;
  };

  // Type-level list of member pointers (including _ign slots)
  // describing the member order of a structural pattern.
  template <auto... Ms>
  struct member_list {};

  // Dependent false for static_assert inside templates.
  template <typename>
  struct dependent_false : std::false_type {};

  // Computes the largest binding position referenced by an
  // expression.
  template <typename E>
  struct max_arg_index;

} // namespace ptn::pat::mod
