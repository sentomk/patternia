#pragma once

/**
 * @file fwd.hpp
 * @brief Forward declarations for Patternia Pattern Base Layer.
 */

namespace ptn::pat::base {

  /** Pattern identification base */
  struct pattern_tag;

  /** The CRTP base class for all pattern implementations */
  template <typename Derived>
  struct pattern_base;

  /** Pattern identification base */
  enum class pattern_kind;

  /** Pattern identification base */
  template <typename Pattern, typename Subject>
  struct binding_args;

  template <typename Pattern, typename Subject>
  using binding_args_t = typename binding_args<Pattern, Subject>::tuple_type;

  /** Pattern identification base */
  // has_match_method
  template <typename P, typename = void>
  struct has_match_method;

  template <typename P>
  inline constexpr bool has_match_method_v = has_match_method<P>::value;

  // has_bind_method
  template <typename P, typename = void>
  struct has_bind_method;

  template <typename P>
  inline constexpr bool has_bind_method_v = has_bind_method<P>::value;

  // is_pattern
  template <typename P>
  struct is_pattern;

  template <typename P>
  inline constexpr bool is_pattern_v = is_pattern<P>::value;

  // pattern_category
  template <typename P, typename = void>
  struct pattern_category;

} // namespace ptn::pat::base
