#pragma once

// Optimization strategy utilities for match case dispatch.

#include <cstddef>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

#include "ptn/core/common/common_traits.hpp"
#include "ptn/core/common/diagnostics.hpp"

namespace ptn::pat::type::detail {
  struct no_subpattern;

  template <typename T, typename SubPattern>
  struct type_is_pattern;

  template <std::size_t I, typename SubPattern>
  struct type_alt_pattern;
} // namespace ptn::pat::type::detail

namespace ptn::pat::mod {
  template <typename Inner, typename Pred>
  struct guarded_pattern;
} // namespace ptn::pat::mod

namespace ptn::pat::detail {
  struct wildcard_t;

  template <typename V, typename Cmp>
  struct literal_pattern;
} // namespace ptn::pat::detail

namespace ptn::core::common {
  namespace detail {
    template <typename T>
    struct is_guarded_pattern : std::false_type {};

    template <typename Inner, typename Pred>
    struct is_guarded_pattern<ptn::pat::mod::guarded_pattern<Inner, Pred>>
        : std::true_type {};

    template <typename T>
    constexpr bool
        is_guarded_pattern_v = is_guarded_pattern<std::decay_t<T>>::value;

    template <typename Lhs, typename Rhs, typename = void>
    struct is_equality_comparable : std::false_type {};

    template <typename Lhs, typename Rhs>
    struct is_equality_comparable<
        Lhs,
        Rhs,
        std::void_t<decltype(std::declval<const Lhs &>()
                             == std::declval<const Rhs &>())>>
        : std::true_type {};

    template <typename Lhs, typename Rhs>
    constexpr bool is_equality_comparable_v =
        is_equality_comparable<Lhs, Rhs>::value;

    // Detects std::variant subjects so fast-path logic only engages on
    // variants.
    template <typename T>
    struct is_variant_subject : std::false_type {};

    template <typename... Alts>
    struct is_variant_subject<std::variant<Alts...>> : std::true_type {};

    template <typename T>
    constexpr bool is_variant_subject_v = is_variant_subject<
        std::remove_cv_t<std::remove_reference_t<T>>>::value;

    // Matches `type::is<T>()` with no subpattern/binding.
    template <typename Pattern>
    struct is_simple_variant_type_is_pattern : std::false_type {};

    template <typename Alt>
    struct is_simple_variant_type_is_pattern<
        ptn::pat::type::detail::
            type_is_pattern<Alt, ptn::pat::type::detail::no_subpattern>>
        : std::true_type {};

    template <typename Pattern>
    constexpr bool
        is_simple_variant_type_is_pattern_v = is_simple_variant_type_is_pattern<
            std::decay_t<Pattern>>::value;

    // Matches `type::alt<I>()` with no subpattern/binding.
    template <typename Pattern>
    struct is_simple_variant_type_alt_pattern : std::false_type {};

    template <std::size_t I>
    struct is_simple_variant_type_alt_pattern<
        ptn::pat::type::detail::
            type_alt_pattern<I, ptn::pat::type::detail::no_subpattern>>
        : std::true_type {};

    template <typename Pattern>
    constexpr bool is_simple_variant_type_alt_pattern_v =
        is_simple_variant_type_alt_pattern<std::decay_t<Pattern>>::value;

    // Matches `type::is<T>(...)` (any subpattern).
    template <typename Pattern>
    struct is_variant_type_is_pattern : std::false_type {};

    template <typename Alt, typename SubPattern>
    struct is_variant_type_is_pattern<
        ptn::pat::type::detail::type_is_pattern<Alt, SubPattern>>
        : std::true_type {};

    template <typename Pattern>
    constexpr bool is_variant_type_is_pattern_v = is_variant_type_is_pattern<
        std::decay_t<Pattern>>::value;

    // Matches `type::alt<I>(...)` (any subpattern).
    template <typename Pattern>
    struct is_variant_type_alt_pattern : std::false_type {};

    template <std::size_t I, typename SubPattern>
    struct is_variant_type_alt_pattern<
        ptn::pat::type::detail::type_alt_pattern<I, SubPattern>>
        : std::true_type {};

    template <typename Pattern>
    constexpr bool is_variant_type_alt_pattern_v = is_variant_type_alt_pattern<
        std::decay_t<Pattern>>::value;

    // Wildcard can act as default arm in dispatcher fast paths.
    template <typename Pattern>
    constexpr bool
        is_wildcard_pattern_v = std::is_same_v<std::decay_t<Pattern>,
                                               ptn::pat::detail::wildcard_t>;

    template <typename Pattern>
    constexpr bool is_simple_variant_dispatch_pattern_v =
        is_simple_variant_type_is_pattern_v<Pattern>
        || is_simple_variant_type_alt_pattern_v<Pattern>
        || is_wildcard_pattern_v<Pattern>;

    // Matches `lit(...)` patterns with default equality comparator.
    template <typename Pattern>
    struct is_simple_literal_pattern : std::false_type {};

    template <typename V>
    struct is_simple_literal_pattern<ptn::pat::detail::literal_pattern<
        V,
        std::equal_to<>>> : std::true_type {};

    template <typename Pattern>
    constexpr bool
        is_simple_literal_pattern_v = is_simple_literal_pattern<
            std::decay_t<Pattern>>::value;

    template <typename Pattern>
    constexpr bool is_simple_literal_dispatch_pattern_v =
        is_simple_literal_pattern_v<Pattern> || is_wildcard_pattern_v<Pattern>;

    template <typename Pattern, typename Subject, bool = false>
    struct is_simple_literal_subject_compatible : std::false_type {};

    template <typename Pattern, typename Subject>
    struct is_simple_literal_subject_compatible<Pattern, Subject, true>
        : std::bool_constant<is_equality_comparable_v<
              Subject,
              typename std::decay_t<Pattern>::store_t>> {};

    // Extracts alt index from `type::alt<I>()` patterns.
    template <typename Pattern>
    struct simple_variant_alt_index;

    template <std::size_t I>
    struct simple_variant_alt_index<ptn::pat::type::detail::type_alt_pattern<
        I,
        ptn::pat::type::detail::no_subpattern>>
        : std::integral_constant<std::size_t, I> {};

    template <typename Pattern>
    struct variant_type_alt_index;

    template <std::size_t I, typename SubPattern>
    struct variant_type_alt_index<
        ptn::pat::type::detail::type_alt_pattern<I, SubPattern>>
        : std::integral_constant<std::size_t, I> {};

    template <typename Pattern>
    struct guarded_inner_pattern;

    template <typename Inner, typename Pred>
    struct guarded_inner_pattern<ptn::pat::mod::guarded_pattern<Inner, Pred>> {
      using type = Inner;
    };

    // Runtime matcher for simple variant patterns by active index.
    template <typename Pattern, typename Subject>
    constexpr bool
    simple_variant_pattern_matches_index(std::size_t active_index) {
      using pattern_t = std::decay_t<Pattern>;

      if constexpr (is_wildcard_pattern_v<pattern_t>) {
        return true;
      }
      else if constexpr (is_simple_variant_type_is_pattern_v<pattern_t>) {
        static_assert_variant_alt_unique<typename pattern_t::alt_t, Subject>();
        return active_index == pattern_t::template alt_index<Subject>();
      }
      else if constexpr (is_simple_variant_type_alt_pattern_v<pattern_t>) {
        constexpr std::size_t I = simple_variant_alt_index<pattern_t>::value;
        static_assert_variant_alt_index<I, Subject>();
        return active_index == I;
      }
      else {
        return false;
      }
    }

    // Compile-time matcher for simple variant patterns by active alternative.
    template <typename Pattern, typename Subject, std::size_t ActiveIndex>
    constexpr bool simple_variant_pattern_matches_alt_index() {
      using pattern_t = std::decay_t<Pattern>;

      if constexpr (is_wildcard_pattern_v<pattern_t>) {
        return true;
      }
      else if constexpr (is_simple_variant_type_is_pattern_v<pattern_t>) {
        static_assert_variant_alt_unique<typename pattern_t::alt_t, Subject>();
        return ActiveIndex == pattern_t::template alt_index<Subject>();
      }
      else if constexpr (is_simple_variant_type_alt_pattern_v<pattern_t>) {
        constexpr std::size_t I = simple_variant_alt_index<pattern_t>::value;
        static_assert_variant_alt_index<I, Subject>();
        return ActiveIndex == I;
      }
      else {
        return false;
      }
    }

    // Runtime index prefilter for mixed variant case chains.
    template <typename Pattern, typename Subject>
    constexpr bool
    variant_pattern_maybe_matches_active_index(std::size_t active_index) {
      using pattern_t = std::decay_t<Pattern>;

      if constexpr (is_wildcard_pattern_v<pattern_t>) {
        return true;
      }
      else if constexpr (is_guarded_pattern_v<pattern_t>) {
        using inner_t = typename guarded_inner_pattern<pattern_t>::type;
        return variant_pattern_maybe_matches_active_index<inner_t, Subject>(
            active_index);
      }
      else if constexpr (is_variant_type_is_pattern_v<pattern_t>) {
        static_assert_variant_alt_unique<typename pattern_t::alt_t, Subject>();
        return active_index == pattern_t::template alt_index<Subject>();
      }
      else if constexpr (is_variant_type_alt_pattern_v<pattern_t>) {
        constexpr std::size_t I = variant_type_alt_index<pattern_t>::value;
        static_assert_variant_alt_index<I, Subject>();
        return active_index == I;
      }
      else {
        return true;
      }
    }

    // Compile-time prefilter by active variant alternative.
    template <typename Pattern, typename Subject, std::size_t ActiveIndex>
    constexpr bool variant_pattern_maybe_matches_alt_index() {
      using pattern_t = std::decay_t<Pattern>;

      if constexpr (is_wildcard_pattern_v<pattern_t>) {
        return true;
      }
      else if constexpr (is_guarded_pattern_v<pattern_t>) {
        using inner_t = typename guarded_inner_pattern<pattern_t>::type;
        return variant_pattern_maybe_matches_alt_index<inner_t,
                                                       Subject,
                                                       ActiveIndex>();
      }
      else if constexpr (is_variant_type_is_pattern_v<pattern_t>) {
        static_assert_variant_alt_unique<typename pattern_t::alt_t, Subject>();
        return ActiveIndex == pattern_t::template alt_index<Subject>();
      }
      else if constexpr (is_variant_type_alt_pattern_v<pattern_t>) {
        constexpr std::size_t I = variant_type_alt_index<pattern_t>::value;
        static_assert_variant_alt_index<I, Subject>();
        return ActiveIndex == I;
      }
      else {
        return true;
      }
    }

    template <typename Case, typename Subject>
    struct is_simple_variant_dispatch_case {
      using case_t       = std::remove_reference_t<Case>;
      using pattern_t    = traits::case_pattern_t<case_t>;
      using handler_t    = traits::case_handler_t<case_t>;
      using bound_args_t = pat::base::binding_args_t<pattern_t, Subject>;

      static constexpr bool
          value = is_simple_variant_dispatch_pattern_v<pattern_t>
                  && std::tuple_size_v<bound_args_t> == 0
                  && std::is_invocable_v<handler_t>;
    };

    template <typename Case, typename Subject>
    struct is_simple_literal_dispatch_case {
      using case_t       = std::remove_reference_t<Case>;
      using pattern_t    = traits::case_pattern_t<case_t>;
      using handler_t    = traits::case_handler_t<case_t>;
      using bound_args_t = pat::base::binding_args_t<pattern_t, Subject>;

      static constexpr bool literal_subject_compatible =
          is_simple_literal_subject_compatible<
              pattern_t,
              Subject,
              is_simple_literal_pattern_v<pattern_t>>::value;

      static constexpr bool value =
          (is_wildcard_pattern_v<pattern_t> || literal_subject_compatible)
          && std::tuple_size_v<bound_args_t> == 0
          && std::is_invocable_v<handler_t>;
    };

    // Fast-path is enabled only when every case qualifies.
    template <typename Subject, typename CasesTuple>
    struct is_simple_variant_dispatch_cases_tuple : std::false_type {};

    template <typename Subject, typename... Cases>
    struct is_simple_variant_dispatch_cases_tuple<Subject, std::tuple<Cases...>>
        : std::bool_constant<(
              is_simple_variant_dispatch_case<Cases, Subject>::value && ...)> {
    };

    template <typename Subject, typename CasesTuple>
    constexpr bool is_simple_variant_dispatch_cases_tuple_v =
        is_simple_variant_dispatch_cases_tuple<
            Subject,
            std::remove_reference_t<CasesTuple>>::value;

    template <typename Subject, typename CasesTuple>
    struct is_simple_literal_dispatch_cases_tuple : std::false_type {};

    template <typename Subject, typename... Cases>
    struct is_simple_literal_dispatch_cases_tuple<Subject, std::tuple<Cases...>>
        : std::bool_constant<(
              is_simple_literal_dispatch_case<Cases, Subject>::value && ...)> {
    };

    template <typename Subject, typename CasesTuple>
    constexpr bool is_simple_literal_dispatch_cases_tuple_v =
        is_simple_literal_dispatch_cases_tuple<
            Subject,
            std::remove_reference_t<CasesTuple>>::value;

    // Central policy entry for dispatch-strategy selection.
    // Future C++20-specific policies should be routed here, so evaluators stay
    // stable.
    template <typename Subject, typename CasesTuple>
    struct dispatch_policy {
      using subject_t = std::remove_cv_t<std::remove_reference_t<Subject>>;

      static constexpr bool use_simple_literal_dispatch =
          (std::is_integral_v<subject_t> || std::is_enum_v<subject_t>)
          && is_simple_literal_dispatch_cases_tuple_v<subject_t, CasesTuple>;

      static constexpr bool use_simple_variant_dispatch =
          is_variant_subject_v<subject_t>
          && is_simple_variant_dispatch_cases_tuple_v<subject_t, CasesTuple>;

      static constexpr bool use_variant_alt_dispatch =
          is_variant_subject_v<subject_t>;
    };

    // Primary template for case-sequence optimization.
    template <typename Subject, typename CasesTuple>
    struct optimize_case_sequence {
      using type = CasesTuple;
    };
  } // namespace detail

  // Convenience alias for optimized case-sequence type.
  // Applies optimization transformations to a case sequence.
  template <typename Subject, typename CasesTuple>
  using optimize_case_sequence_t =
      typename detail::optimize_case_sequence<Subject, CasesTuple>::type;

} // namespace ptn::core::common
