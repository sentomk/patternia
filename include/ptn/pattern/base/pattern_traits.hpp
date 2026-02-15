#pragma once

// Compile-time detection utilities for Patternia patterns.
//
// This file provides type traits for detecting and characterizing patterns,
// including pattern identification, binding pattern detection, structural
// pattern traits, and guard predicate traits.

#include <type_traits>
#include <utility>

#include "ptn/pattern/modifiers/fwd.h"
#include "ptn/pattern/base/pattern_base.hpp"

namespace ptn::pat::traits {

  // -----------------------------------------------------------------------
  // Pattern Identification
  // -----------------------------------------------------------------------

  // Detects if a type has a .match(Subject) method.
  template <typename P, typename = void>
  struct has_match_method : std::false_type {};

  template <typename P>
  struct has_match_method<
      P,
      std::void_t<decltype(static_cast<bool>(std::declval<const P &>().match(
          std::declval<int>())))>> : std::true_type {};

  // Variable template for has_match_method<P>::value.
  template <typename P>
  inline constexpr bool has_match_method_v = has_match_method<P>::value;

  // Trait: determines whether P acts as a Pattern.
  //
  // A type is considered a pattern if it inherits from base::pattern_tag
  // OR has a .match(Subject) method.
  template <typename P>
  struct is_pattern
      : std::integral_constant<
            bool,
            std::is_base_of_v<base::pattern_tag, P> || has_match_method_v<P>> {
  };

  // Convenience variable template for is_pattern<P>::value.
  template <typename P>
  inline constexpr bool is_pattern_v = is_pattern<P>::value;

  // -----------------------------------------------------------------------
  // Binding Pattern Traits
  // -----------------------------------------------------------------------

  // Detects if a pattern is a binding pattern.
  //
  // Binding patterns are those that inherit from binding_pattern_base and
  // have a static is_binding member.
  template <typename P, typename = void>
  struct is_binding_pattern : std::false_type {};

  // Specialization for binding patterns.
  template <typename P>
  struct is_binding_pattern<P, std::void_t<decltype(P::is_binding)>>
      : std::bool_constant<P::is_binding> {};

  // Helper variable template for is_binding_pattern.
  template <typename P>
  inline constexpr bool is_binding_pattern_v = is_binding_pattern<P>::value;

  // -----------------------------------------------------------------------
  // Structural Pattern Traits
  // -----------------------------------------------------------------------

  // Checks if M is a non-static data member pointer.
  template <auto M>
  inline constexpr bool is_data_member_ptr_v =
      std::is_member_object_pointer_v<decltype(M)>;

  // Checks if M is a nullptr placeholder (e.g., _ign).
  template <auto M>
  inline constexpr bool is_nullptr_placeholder_v =
      std::is_same_v<std::decay_t<decltype(M)>, std::nullptr_t>;

  // Unified notion: M is a structural element.
  //
  // Structural elements are either data member pointers or nullptr placeholders
  // used in has<> patterns.
  template <auto M>
  inline constexpr bool is_structural_element_v =
      is_data_member_ptr_v<M> || is_nullptr_placeholder_v<M>;

  // -----------------------------------------------------------------------
  // Guard Predicate Traits
  // -----------------------------------------------------------------------

  // Marker tag for guard predicates.
  //
  // Guard predicates should inherit from this tag to enable detection.
  struct guard_predicate_tag {};

  // Detects if a type is a guard predicate.
  //
  // A type is considered a guard predicate if it inherits from
  // guard_predicate_tag.
  template <typename T>
  inline constexpr bool is_guard_predicate_v =
      std::is_base_of_v<guard_predicate_tag, std::decay_t<T>>;

  // Trait to detect argument expression nodes.
  //
  // Argument expressions include placeholders (arg_t), value wrappers (val_t),
  // binary expressions (bin_expr), and unary expressions (un_expr).
  template <typename T>
  struct is_arg_expr : std::false_type {};

  template <std::size_t I>
  struct is_arg_expr<mod::arg_t<I>> : std::true_type {};

  template <typename T>
  struct is_arg_expr<mod::val_t<T>> : std::true_type {};

  template <typename Op, typename L, typename R>
  struct is_arg_expr<mod::bin_expr<Op, L, R>> : std::true_type {};

  template <typename Op, typename X>
  struct is_arg_expr<mod::un_expr<Op, X>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_arg_expr_v = is_arg_expr<std::decay_t<T>>::value;

  // Trait to detect tuple predicates.
  //
  // Tuple predicates wrap expression templates and operate on bound tuples.
  template <typename T>
  struct is_tuple_predicate : std::false_type {};

  template <typename E>
  struct is_tuple_predicate<mod::tuple_predicate<E>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_tuple_predicate_v =
      is_tuple_predicate<std::decay_t<T>>::value;

  // Trait to detect tuple guard predicates (including && / || compositions).
  //
  // Tuple guard predicates are predicates that can be called with a tuple
  // of bound values. This includes tuple_predicate and logical compositions
  // (pred_and, pred_or) of tuple guard predicates.
  template <typename T>
  struct is_tuple_guard_predicate : std::false_type {};

  template <typename E>
  struct is_tuple_guard_predicate<mod::tuple_predicate<E>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_tuple_guard_predicate_v =
      is_tuple_guard_predicate<std::decay_t<T>>::value;

  template <typename L, typename R>
  struct is_tuple_guard_predicate<mod::pred_and<L, R>>
      : std::bool_constant<
            is_tuple_guard_predicate_v<L> || is_tuple_guard_predicate_v<R>> {};

  template <typename L, typename R>
  struct is_tuple_guard_predicate<mod::pred_or<L, R>>
      : std::bool_constant<
            is_tuple_guard_predicate_v<L> || is_tuple_guard_predicate_v<R>> {};

} // namespace ptn::pat::traits
