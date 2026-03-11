#pragma once

// Forward declarations for Patternia Core Layer.
//
// This header contains forward declarations to avoid circular dependencies
// and provide minimal interface information for the core components.

namespace ptn {

  // Public API match entry.
  template <typename T>
  constexpr auto match(T &value);

} // namespace ptn
