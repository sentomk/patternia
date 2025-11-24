#pragma once

#include "ptn/config.hpp"
#include "ptn/core/detail/match_builder_detail.hpp"

/**
 * @file match_builder.hpp
 * @brief Public API for the Core Layer matching engine.
 *
 * This header provides the public interface to Patternia's *Core Layer*.
 * The actual implementation is in `ptn/core/detail/match_builder_impl.hpp`.
 *
 * `match_builder` owns:
 *   - a subject value
 *   - a compile-time tuple of (Pattern, Handler) pairs
 *
 * The builder supports:
 *   - `.when(Pattern >> Handler)` to append a new case
 *   - `.otherwise(handler)` to perform final evaluation
 *
 * @ingroup core
 */

namespace ptn {

  /**
   * @brief Forward declaration of free-function `match()`.
   */
  template <typename T>
  constexpr auto
  match(T &&) noexcept(std::is_nothrow_constructible_v<std::decay_t<T>, T &&>);

} // namespace ptn

namespace ptn::core {

  // Forward declare the detail namespace
  namespace detail {
    template <typename TV, typename... Cases>
    class match_builder;
  }

  /**
   * @brief Public type alias forwarding to the detail implementation.
   *
   * Users see `ptn::core::match_builder<TV, Cases...>` which is actually
   * `ptn::core::detail::match_builder<TV, Cases...>` internally.
   */
  template <typename TV, typename... Cases>
  using match_builder = detail::match_builder<TV, Cases...>;

} // namespace ptn::core
