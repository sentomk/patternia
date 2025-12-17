#pragma once

// Forward declarations for Patternia Pattern Mod Layer.

#include <cstddef>

namespace ptn::pat::mod {

  // Forward declare guarded pattern for use in guard_operator
  template <typename Inner, typename Pred>
  struct guarded_pattern;

  // Guard predicate tag - forward declaration for traits
  struct guard_predicate_tag;

  // Forward declare tuple predicate and related types
  template <typename Expr>
  struct tuple_predicate;

  template <typename L, typename R>
  struct pred_and;

  template <typename L, typename R>
  struct pred_or;

  // Forward declare guard predicate detection traits
  template <typename T>
  struct is_tuple_predicate;

  template <typename T>
  struct is_tuple_guard_predicate;

  // Forward declare expression template types
  template <std::size_t I>
  struct arg_t;

  template <typename T>
  struct val_t;

  template <typename Op, typename L, typename R>
  struct bin_expr;

  template <typename Op, typename X>
  struct un_expr;

  // Forward declare expression traits
  template <typename T>
  struct is_arg_expr;

  // Forward declare max argument index traits
  template <typename E>
  struct max_arg_index;

  // Forward declare max tuple guard index traits
  template <typename T>
  struct max_tuple_guard_index;

  // Forward declare placeholder type
  struct placeholder_t;

  // Forward declare range predicate and related types
  enum class range_mode : int;

  struct closed_t;
  struct open_t;
  struct open_closed_t;
  struct closed_open_t;

  template <typename T>
  struct range_predicate;

  // Forward declare binary predicate
  template <typename Op, typename RHS>
  struct binary_predicate;

} // namespace ptn::pat::mod
