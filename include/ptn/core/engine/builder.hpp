#pragma once

/**
 * @file builder.hpp
 * @brief Public-facing builder type for the Core matching engine.
 *
 * Internally, it is an alias to `ptn::core::detail::match_builder`.
 */

#include "ptn/config.hpp"

// Pull in the implementation
#include "ptn/core/engine/detail/builder_impl.hpp"

namespace ptn::core {

  namespace detail {

    /**
     * @brief Forward declaration of the concrete builder implementation.
     *
     * TV    : subject type
     * Cases : sequence of case_expr-like types
     */
    template <typename TV, typename... Cases>
    class match_builder;

  } // namespace detail

  /**
   * @brief Public alias forwarding to the actual implementation.
   */
  template <typename TV, typename Cases>
  using match_builder = detail::match_builder<TV, Cases>;

} // namespace ptn::core
