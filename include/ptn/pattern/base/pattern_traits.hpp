#pragma once
#include <type_traits>
#include <utility>

#include "ptn//pattern/modifiers/fwd.h"

// Compile-time detection utilities for Patternia patterns.

namespace ptn::pat::traits {

  /* ---------- IS A PATTERN? ----------*/
  /* ---------------------------------- */
  /* ---------------------------------- */

  /* START */
  // Pattern identification base
  template <typename P, typename = void>
  struct has_match_method : std::false_type {};

  template <typename P>
  struct has_match_method<
      P,
      std::void_t<decltype(static_cast<bool>(std::declval<const P &>().match(
          std::declval<int>())))>> : std::true_type {};

  // Pattern identification base

  // Marker base type for all built-in Patternia patterns.
  struct pattern_tag {
    static constexpr bool is_pattern = true;
  };

  // Pattern identification base
  // Trait: determines whether P acts as a Pattern.
  template <typename P>
  struct is_pattern : std::integral_constant<
                          bool,
                          std::is_base_of_v<pattern_tag, P> ||
                              ptn::pat::traits::has_match_method<P>::value> {};

  // Convenience variable template for is_pattern<P>::value.
  template <typename P>
  inline constexpr bool is_pattern_v = is_pattern<P>::value;

  /* END */
  /* ---------------------------------- */
  /* ---------------------------------- */

  /* -------------------------------------------- */
  /* ---------- Binding Pattern Traits ---------- */
  /* -------------------------------------------- */

  /* START */
  // Detects if a pattern is a binding pattern.
  template <typename P, typename = void>
  struct is_binding_pattern : std::false_type {};

  // Specialization for binding patterns.
  template <typename P>
  struct is_binding_pattern<P, std::void_t<decltype(P::is_binding)>>
      : std::bool_constant<P::is_binding> {};

  // Helper variable template for is_binding_pattern.
  template <typename P>
  inline constexpr bool is_binding_pattern_v = is_binding_pattern<P>::value;

  /* END */
  /* -------------------------------------------- */
  /* -------------------------------------------- */

  /* ----------------------------------------------- */
  /* ---------- Structural Pattern Traits ---------- */
  /* ----------------------------------------------- */

  /* START*/
  template <auto M>
  inline constexpr bool is_data_member_ptr_v =
      std::is_member_object_pointer_v<decltype(M)>;

  // C++17 fallback placeholder (rest)
  template <auto M>
  inline constexpr bool is_nullptr_placeholder_v =
      std::is_same_v<std::decay_t<decltype(M)>, std::nullptr_t>;

  // Unified notion: structural element
  template <auto M>
  inline constexpr bool is_structural_element_v =
      is_data_member_ptr_v<M> || is_nullptr_placeholder_v<M>;

  /* END */
  /* ----------------------------------------------- */
  /* ----------------------------------------------- */

  /* ---------- Guard Traits ---------- */
  /* ---------------------------------- */
  /* ---------------------------------- */

  /* START */
  // Marker tag for guard predicates.
  struct guard_predicate_tag {};

  template <typename T>
  inline constexpr bool is_guard_predicate_v =
      std::is_base_of_v<guard_predicate_tag, std::decay_t<T>>;

  // Trait to detect argument expression nodes.
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
  template <typename T>
  struct is_tuple_predicate : std::false_type {};

  template <typename E>
  struct is_tuple_predicate<mod::tuple_predicate<E>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_tuple_predicate_v =
      is_tuple_predicate<std::decay_t<T>>::value;

  // Trait to detect tuple guard predicates (including && / || compositions).
  template <typename T>
  struct is_tuple_guard_predicate : std::false_type {};

  template <typename E>
  struct is_tuple_guard_predicate<mod::tuple_predicate<E>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_tuple_guard_predicate_v =
      is_tuple_guard_predicate<std::decay_t<T>>::value;

  // Tuple guard predicate composition.
  template <typename L, typename R>
  struct is_tuple_guard_predicate<mod::pred_and<L, R>>
      : std::bool_constant<
            is_tuple_guard_predicate_v<L> || is_tuple_guard_predicate_v<R>> {};

  template <typename L, typename R>
  struct is_tuple_guard_predicate<mod::pred_or<L, R>>
      : std::bool_constant<
            is_tuple_guard_predicate_v<L> || is_tuple_guard_predicate_v<R>> {};

  /* END */
  /* ---------------------------------- */
  /* ---------------------------------- */

} // namespace ptn::pat::traits
