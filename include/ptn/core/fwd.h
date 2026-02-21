#pragma once

// Forward declarations for Patternia Core Layer.
//
// This header contains forward declarations to avoid circular dependencies
// and provide minimal interface information for the core components.

namespace ptn {

  namespace core::engine::detail {
    template <typename TV, bool HasMatchFallback, typename... Cases>
    class match_builder;
  } // namespace core::engine::detail

  // Public API match entry.
  template <typename T>
  constexpr auto match(T &value);

} // namespace ptn
