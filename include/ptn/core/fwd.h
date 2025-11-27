#pragma once
/**
 * @file fwd.hpp
 * @brief Forward declarations for Patternia Core Layer.
 */

namespace ptn {
  /* Entry function: match() */
  template <typename T>
  constexpr auto match(T &&);

  template <typename U, typename T>
  constexpr auto match(T &&);

  /* Core builder type (public-facing alias) */
  namespace core {

    /**
     * @brief Public-facing builder template.
     *
     * Actual implementation is in namespace ptn::core::detail::match_builder.
     */
    template <typename TV, typename... Cases>
    class match_builder; // only forward declared here

  } // namespace core

  /* Core internals (detail) */
  namespace core::detail {

    /**
     * @brief Internal builder implementation.
     */
    template <typename TV, typename... Cases>
    class match_builder;

    /**
     * @brief Single-case evaluation (pattern + handler).
     */
    template <typename Case, typename Subject>
    struct case_eval;

    /**
     * @brief Full evaluation logic for the match builder.
     */
    struct match_impl;

  } // namespace core::detail

} // namespace ptn