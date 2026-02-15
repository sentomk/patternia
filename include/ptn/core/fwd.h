#pragma once

// Forward declarations for Patternia Core Layer.
//
// This header contains forward declarations to avoid circular dependencies
// and provide minimal interface information for the core components.

namespace ptn {

  namespace core::dsl::detail {
    template <typename Pattern, typename Handler>
    struct case_expr;
    template <typename... Cases>
    struct cases_pack;
  } // namespace core::dsl::detail

  namespace core::engine::detail {
    template <typename TV, bool HasMatchFallback, typename... Cases>
    class match_builder;
  } // namespace core::engine::detail

  namespace core::engine {
    template <typename TV, typename... Cases>
    constexpr auto match(TV &subject, core::dsl::detail::cases_pack<Cases...>);
  } // namespace core::engine

  // Public API match entry.
  template <typename T>
  constexpr auto match(T &value);

} // namespace ptn
