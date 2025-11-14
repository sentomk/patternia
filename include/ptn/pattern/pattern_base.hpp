#pragma once

#include "ptn/pattern/pattern_traits.hpp" // marker
#include "ptn/config.hpp"
#include <utility>

/**
 * @file pattern_base.hpp
 * @brief CRTP base class providing unified pattern interface for Patternia.
 *
 * All built-in patterns should derive from `pattern_base<Derived>`.
 * This base class delivers:
 *   - the “pattern identity” (via pattern_tag)
 *   - default forwarding for `match(subject)`
 *   - default forwarding for `bind(subject)`
 *   - default no-op bind for predicate-like patterns
 *
 * Part of Pattern Layer (ptn::pattern)
 */

namespace ptn::pattern {

  /* forward declare */
  template <typename Derived>
  struct pattern_base;

  /// Helper to obtain Derived&
  template <typename Derived>
  constexpr Derived &derived_of(pattern_base<Derived> &self) noexcept {
    return static_cast<Derived &>(self);
  }
  /// Helper to obtain const Derived&
  template <typename Derived>
  constexpr const Derived &
  derived_of(const pattern_base<Derived> &self) noexcept {
    return static_cast<const Derived &>(self);
  }

  /**
   * @brief CRTP base class for all Patternia patterns.
   *
   * Derived must implement:
   *   - `bool match(const Subject&) const`
   *   - optionally: `auto bind(const Subject&) const`
   *
   * If bind() is not implemented, pattern_base returns subject unchanged.
   */
  template <typename Derived>
  struct pattern_base : detail::pattern_tag {

    /**
     * @brief default match() forwarding to Derived::match()
     */
    template <typename Subject>
    constexpr bool match(Subject &&subj) const noexcept(
        noexcept(derived_of(*this).match(std::forward<Subject>(subj)))) {
      return derived_of(*this).match(std::forward<Subject>(subj));
    }

    /**
     * @brief Default bind(): return the subject unchanged.
     *
     * Predicate patterns or patterns that do not transform the value may
     * simply rely on this implementation.
     *
     * Value-pattern, type-pattern etc should override bind().
     */
    template <typename Subject>
    constexpr bool bind(Subject &&subj) const noexcept {
      return std::forward<Subject>(subj);
    }
  };

} // namespace ptn::pattern