// Public API and implementation for type-based patterns
// (`is<T>()`). Provides variant alternative matching by type,
// with optional sub-pattern matching for binding or structural
// checks.

#include <tuple>
#include <type_traits>
#include <utility>

#include "ptn/core/common/diagnostics.hpp"
#include "ptn/meta/base/traits.hpp"
#include "ptn/meta/query/index.hpp"
#include "ptn/meta/query/template_info.hpp"
#include "ptn/pattern/bind.hpp"
#include "ptn/pattern/base/fwd.h"
#include "ptn/pattern/base/binding_base.hpp"
#include "ptn/pattern/base/pattern_base.hpp"

namespace ptn::pat::detail {

    // Sentinel type representing "no subpattern".
    struct no_subpattern {};

    // Empty base used when a pattern is not a binding pattern.
    struct non_binding_base {};

    // Matches a specific variant alternative by type, with optional
    // subpattern.
    // - When SubPattern is no_subpattern: only checks the
    // alternative.
    // - Otherwise: delegates match/bind to the subpattern on the
    // selected alt.
    //
    // Important: when SubPattern is a binding pattern (e.g. bind(),
    // bind(has<...>())), this type pattern itself becomes a binding
    // pattern, enabling guard syntax:
    //   $(is<T>())[predicate]   // equivalent to
    //   is<T>(bind()[predicate])
    template <typename T, typename SubPattern = no_subpattern>
    struct type_is_pattern
        : base::pattern_base<type_is_pattern<T, SubPattern>>,
          std::conditional_t<
              ptn::pat::traits::is_binding_pattern_v<SubPattern>,
              base::binding_pattern_base<
                  type_is_pattern<T, SubPattern>>,
              non_binding_base> {
      using alt_t = meta::remove_cvref_t<T>;

      SubPattern subpattern;

      constexpr type_is_pattern() = default;
      constexpr explicit type_is_pattern(SubPattern sub)
          : subpattern(std::move(sub)) {
      }

      // Computes the alternative index of alt_t within Subject's
      // variant.
      template <typename Subject>
      static constexpr std::size_t alt_index() {
        using subject_t = meta::remove_cvref_t<Subject>;
        using args_t = typename meta::template_info<subject_t>::args;
        // Map alternative type to its variant index using meta
        // type_list.
        constexpr int  idx = meta::index_of_v<alt_t, args_t>;
        constexpr bool alt_type_found = (idx >= 0);
        static_assert(
            alt_type_found,
            "[Patternia.is]: Alternative type not found in "
            "std::variant. Tip: ensure the variant lists T, or use "
            "alt<I>() to match by index.");
        return static_cast<std::size_t>(idx);
      }

      template <typename Subject>
      constexpr bool match(const Subject &subject) const noexcept {
        core::common::static_assert_variant_alt_unique<alt_t,
                                                       Subject>();
        // Fast alternative check via index; subpattern evaluated
        // only on hit.
        if (subject.index() != alt_index<Subject>())
          return false;
        if constexpr (std::is_same_v<SubPattern, no_subpattern>) {
          return true;
        }
        else {
          return subpattern.match(
              std::get<alt_index<Subject>()>(subject));
        }
      }

      template <typename Subject>
      constexpr decltype(auto) bind(const Subject &subject) const {
        core::common::static_assert_variant_alt_unique<alt_t,
                                                       Subject>();
        // No bindings when there is no subpattern.
        if constexpr (std::is_same_v<SubPattern, no_subpattern>) {
          return std::tuple<>{};
        }
        else {
          return subpattern.bind(
              std::get<alt_index<Subject>()>(subject));
        }
      }
    };

    // Matches a specific alternative by index, with optional
    // subpattern.
    // - When SubPattern is no_subpattern: only checks the index.
    // - Otherwise: delegates match/bind to the subpattern on the
    // selected alt.
    //
    // Like type_is_pattern, this becomes a binding pattern when
    // SubPattern binds, so guards can be attached directly to the
    // type pattern.
    template <std::size_t I, typename SubPattern = no_subpattern>
    struct type_alt_pattern
        : base::pattern_base<type_alt_pattern<I, SubPattern>>,
          std::conditional_t<
              ptn::pat::traits::is_binding_pattern_v<SubPattern>,
              base::binding_pattern_base<
                  type_alt_pattern<I, SubPattern>>,
              non_binding_base> {
      SubPattern subpattern;

      constexpr type_alt_pattern() = default;
      constexpr explicit type_alt_pattern(SubPattern sub)
          : subpattern(std::move(sub)) {
      }

      template <typename Subject>
      constexpr bool match(const Subject &subject) const noexcept {
        core::common::static_assert_variant_alt_index<I, Subject>();
        // Fast alternative check via index; subpattern evaluated
        // only on hit.
        if (subject.index() != I)
          return false;
        if constexpr (std::is_same_v<SubPattern, no_subpattern>) {
          return true;
        }
        else {
          return subpattern.match(std::get<I>(subject));
        }
      }

      template <typename Subject>
      constexpr decltype(auto) bind(const Subject &subject) const {
        core::common::static_assert_variant_alt_index<I, Subject>();
        // No bindings when there is no subpattern.
        if constexpr (std::is_same_v<SubPattern, no_subpattern>) {
          return std::tuple<>{};
        }
        else {
          return subpattern.bind(std::get<I>(subject));
        }
      }
    };

  } // namespace ptn::pat::detail

// ----------------------------------------------------------------
// Short-form variable template aliases.
//
// These live in ptn::pat so they can be exported directly into ptn::
// without a namespace prefix.
//
// Usage:
//   is<int>                     // match variant alt, no bind
//   is<int>(sub_pattern)        // match + delegate to sub_pattern
//   $(is<int>())                // match + bind value (use $
//   callable)
//   $(is<int>())[_0 > 0]        // match + bind + guard
//   alt_idx<0>                  // match by index, no bind
// ----------------------------------------------------------------

namespace ptn::pat {

  namespace detail {

    // Factory object for is<T> variable template.
    //
    // Acts as a pattern itself (delegates to type_is_pattern<T>),
    // and supports operator() to attach a sub-pattern.
    template <typename T>
    struct is_factory : type_is_pattern<T> {

      constexpr is_factory() = default;

      // No-arg call form: `is<T>()`.
      constexpr type_is_pattern<T> operator()() const {
        return {};
      }

      // Attach a sub-pattern: is<T>(sub)
      template <typename SubPattern>
      constexpr auto operator()(SubPattern &&sub) const {
        return type_is_pattern<T, std::decay_t<SubPattern>>(
                std::forward<SubPattern>(sub));
      }
    };

    // Factory object for alt<I> variable template.
    template <std::size_t I>
    struct alt_factory : type_alt_pattern<I> {

      constexpr alt_factory() = default;

      // No-arg call form: `alt<I>()`.
      constexpr type_alt_pattern<I>
      operator()() const {
        return {};
      }

      template <typename SubPattern>
      constexpr auto operator()(SubPattern &&sub) const {
        return type_alt_pattern<I, std::decay_t<SubPattern>>(
                std::forward<SubPattern>(sub));
      }
    };

  } // namespace detail

  // Variable template: match variant alternative by type (no bind).
  template <typename T>
  inline constexpr detail::is_factory<T> is{};

  // Variable template: match variant alternative by index (no bind).
  template <std::size_t I>
  inline constexpr detail::alt_factory<I> alt{};

} // namespace ptn::pat

// ------------------------------------------------------------
// Binding Contract Declaration
// ------------------------------------------------------------

namespace ptn::pat::base {

  template <typename T, typename Subject>
  struct binding_args<ptn::pat::detail::type_is_pattern<
                          T,
                          ptn::pat::detail::no_subpattern>,
                      Subject> {
    using type = std::tuple<>;
  };

  template <typename T, typename SubPattern, typename Subject>
  struct binding_args<
      ptn::pat::detail::type_is_pattern<T, SubPattern>,
      Subject> {
    using alt_t = ptn::meta::remove_cvref_t<T>;
    using type  = typename binding_args<SubPattern, alt_t>::type;
  };

  template <std::size_t I, typename Subject>
  struct binding_args<ptn::pat::detail::type_alt_pattern<
                          I,
                          ptn::pat::detail::no_subpattern>,
                      Subject> {
    using type = std::tuple<>;
  };

  template <std::size_t I, typename SubPattern, typename Subject>
  struct binding_args<
      ptn::pat::detail::type_alt_pattern<I, SubPattern>,
      Subject> {
    using subject_t = ptn::meta::remove_cvref_t<Subject>;
    using args_t    = typename ptn::meta::template_info<
           subject_t>::args;
    // Derive alternative type from meta type_list instead of
    // std::variant_alternative.
    using alt_t = ptn::meta::index_t<I, args_t>;
    using type  = typename binding_args<SubPattern, alt_t>::type;
  };

} // namespace ptn::pat::base

// binding_args specializations for factory types.
namespace ptn::pat::base {

  // is_factory<T> delegates to type_is_pattern<T, no_subpattern>.
  template <typename T, typename Subject>
  struct binding_args<ptn::pat::detail::is_factory<T>, Subject> {
    using type = std::tuple<>;
  };

  // alt_factory<I> delegates to type_alt_pattern<I, no_subpattern>.
  template <std::size_t I, typename Subject>
  struct binding_args<ptn::pat::detail::alt_factory<I>, Subject> {
    using type = std::tuple<>;
  };

} // namespace ptn::pat::base
