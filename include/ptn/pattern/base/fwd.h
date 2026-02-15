#pragma once

// Forward declarations for Patternia Pattern Base Layer.

namespace ptn::pat::base {

  // Pattern identification base
  struct pattern_tag;

  // The CRTP base class for all pattern implementations
  template <typename Derived>
  struct pattern_base;

  // Pattern identification base
  enum class pattern_kind;

  // Pattern identification base
  template <typename Pattern, typename Subject>
  struct binding_args;

  template <typename Pattern, typename Subject>
  using binding_args_t = typename binding_args<Pattern, Subject>::type;

  template <typename Derived>
  struct binding_pattern_base;

} // namespace ptn::pat::base
