#pragma once

// Optimization strategy utilities for match case dispatch.

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

#include "ptn/core/common/common_traits.hpp"
#include "ptn/core/common/diagnostics.hpp"

namespace ptn::pat::detail {
  struct no_subpattern;

  template <typename T, typename SubPattern>
  struct type_is_pattern;

  template <std::size_t I, typename SubPattern>
  struct type_alt_pattern;
} // namespace ptn::pat::detail

namespace ptn::pat::mod {
  template <typename Inner, typename Pred>
  struct guarded_pattern;
} // namespace ptn::pat::mod

namespace ptn::pat::detail {
  struct binding_pattern;

  struct wildcard_t;

  template <typename V, typename Cmp>
  struct literal_pattern;

  template <auto V, typename Cmp>
  struct static_literal_pattern;
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

    template <typename T, bool = std::is_enum_v<T>>
    struct literal_subject_key {
      using type = T;
    };

    template <typename T>
    struct literal_subject_key<T, true> {
      using type = std::underlying_type_t<T>;
    };

    template <typename T>
    using literal_subject_key_t = typename literal_subject_key<T>::type;

    // Detects std::variant subjects so lowering only engages on variants.
    template <typename T>
    struct is_variant_subject : std::false_type {};

    template <typename... Alts>
    struct is_variant_subject<std::variant<Alts...>> : std::true_type {};

    template <typename T>
    constexpr bool is_variant_subject_v = is_variant_subject<
        std::remove_cv_t<std::remove_reference_t<T>>>::value;

    constexpr std::size_t k_variant_inline_dispatch_alt_threshold = 16;
    constexpr std::size_t k_variant_segmented_dispatch_alt_threshold = 64;
    constexpr std::size_t k_variant_dispatch_segment_size = 16;
    constexpr std::size_t k_static_literal_dense_dispatch_max_cases = 256;
    constexpr std::size_t k_static_literal_dense_dispatch_max_span = 512;
    constexpr std::size_t k_static_literal_dense_dispatch_max_density = 4;

    enum class variant_dispatch_tier {
      hot_inline,
      warm_segmented,
      cold_compact
    };

    template <std::size_t AltCount>
    constexpr variant_dispatch_tier variant_dispatch_tier_for_alt_count() {
      if constexpr (AltCount <= k_variant_inline_dispatch_alt_threshold) {
        return variant_dispatch_tier::hot_inline;
      }
      else if constexpr (AltCount <= k_variant_segmented_dispatch_alt_threshold) {
        return variant_dispatch_tier::warm_segmented;
      }
      else {
        return variant_dispatch_tier::cold_compact;
      }
    }

    // Compile-time binary dispatcher for variant alternative indexes.
    // Keeps lowering analysis in optimize.hpp while evaluators provide
    // concrete on-hit / on-miss behavior.
    template <std::size_t BeginAlt,
              std::size_t EndAlt,
              std::size_t AltCount>
    struct variant_binary_alt_dispatch {
      template <typename OnHit, typename OnMiss>
      static constexpr decltype(auto)
      dispatch(std::size_t active_index, OnHit &&on_hit, OnMiss &&on_miss) {
        if constexpr (BeginAlt >= EndAlt || BeginAlt >= AltCount) {
          return std::forward<OnMiss>(on_miss)();
        }
        else if constexpr ((EndAlt - BeginAlt) == 1) {
          if (active_index == BeginAlt) {
            return std::forward<OnHit>(on_hit)(
                std::integral_constant<std::size_t, BeginAlt>{});
          }

          return std::forward<OnMiss>(on_miss)();
        }
        else {
          constexpr std::size_t
              mid = BeginAlt + ((EndAlt - BeginAlt) / 2);

          if (active_index < mid) {
            return variant_binary_alt_dispatch<BeginAlt,
                                               mid,
                                               AltCount>::dispatch(
                active_index,
                std::forward<OnHit>(on_hit),
                std::forward<OnMiss>(on_miss));
          }

          return variant_binary_alt_dispatch<mid,
                                             EndAlt,
                                             AltCount>::dispatch(
              active_index,
              std::forward<OnHit>(on_hit),
              std::forward<OnMiss>(on_miss));
        }
      }
    };

    // Matches `is<T>()` with no subpattern/binding.
    template <typename Pattern>
    struct is_simple_variant_type_is_pattern : std::false_type {};

    template <typename Alt>
    struct is_simple_variant_type_is_pattern<
        ptn::pat::detail::
            type_is_pattern<Alt, ptn::pat::detail::no_subpattern>>
        : std::true_type {};

    template <typename Pattern>
    constexpr bool
        is_simple_variant_type_is_pattern_v = is_simple_variant_type_is_pattern<
            std::decay_t<Pattern>>::value;

    // Matches `alt<I>()` with no subpattern/binding.
    template <typename Pattern>
    struct is_simple_variant_type_alt_pattern : std::false_type {};

    template <std::size_t I>
    struct is_simple_variant_type_alt_pattern<
        ptn::pat::detail::
            type_alt_pattern<I, ptn::pat::detail::no_subpattern>>
        : std::true_type {};

    template <typename Pattern>
    constexpr bool is_simple_variant_type_alt_pattern_v =
        is_simple_variant_type_alt_pattern<std::decay_t<Pattern>>::value;

    // Matches `is<T>(...)` (any subpattern).
    template <typename Pattern>
    struct is_variant_type_is_pattern : std::false_type {};

    template <typename Alt, typename SubPattern>
    struct is_variant_type_is_pattern<
        ptn::pat::detail::type_is_pattern<Alt, SubPattern>>
        : std::true_type {};

    template <typename Pattern>
    constexpr bool is_variant_type_is_pattern_v = is_variant_type_is_pattern<
        std::decay_t<Pattern>>::value;

    // Matches `alt<I>(...)` (any subpattern).
    template <typename Pattern>
    struct is_variant_type_alt_pattern : std::false_type {};

    template <std::size_t I, typename SubPattern>
    struct is_variant_type_alt_pattern<
        ptn::pat::detail::type_alt_pattern<I, SubPattern>>
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

    // Matches runtime literal_pattern with default equality comparator.
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
    struct is_static_literal_pattern : std::false_type {};

    template <auto V>
    struct is_static_literal_pattern<ptn::pat::detail::static_literal_pattern<
        V,
        std::equal_to<>>> : std::true_type {};

    template <typename Pattern>
    constexpr bool
        is_static_literal_pattern_v = is_static_literal_pattern<
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

    template <typename Pattern, typename Subject, bool = false>
    struct is_static_literal_subject_compatible : std::false_type {};

    template <typename Pattern, typename Subject>
    struct is_static_literal_subject_compatible<Pattern, Subject, true>
        : std::bool_constant<
              is_equality_comparable_v<
                  Subject,
                  typename std::decay_t<Pattern>::store_t>
              && std::is_constructible_v<
                  literal_subject_key_t<
                      std::remove_cv_t<std::remove_reference_t<Subject>>>,
                  decltype(std::decay_t<Pattern>::value)>> {};

    template <typename Pattern, typename Subject>
    struct static_literal_case_value;

    template <auto V, typename Subject>
    struct static_literal_case_value<
        ptn::pat::detail::static_literal_pattern<V, std::equal_to<>>,
        Subject> {
      using subject_t = std::remove_cv_t<std::remove_reference_t<Subject>>;
      using key_t     = literal_subject_key_t<subject_t>;

      inline static constexpr key_t value = static_cast<key_t>(V);
    };

    // Extracts alt index from `alt<I>()` patterns.
    template <typename Pattern>
    struct simple_variant_alt_index;

    template <std::size_t I>
    struct simple_variant_alt_index<ptn::pat::detail::type_alt_pattern<
        I,
        ptn::pat::detail::no_subpattern>>
        : std::integral_constant<std::size_t, I> {};

    template <typename Pattern>
    struct variant_type_alt_index;

    template <std::size_t I, typename SubPattern>
    struct variant_type_alt_index<
        ptn::pat::detail::type_alt_pattern<I, SubPattern>>
        : std::integral_constant<std::size_t, I> {};

    // Matches `$(is<T>())`.
    template <typename Pattern>
    struct is_variant_direct_ref_bind_pattern : std::false_type {};

    template <typename Alt>
    struct is_variant_direct_ref_bind_pattern<
        ptn::pat::detail::type_is_pattern<Alt,
                                                ptn::pat::detail::binding_pattern>>
        : std::true_type {};

    // Matches `$(alt<I>())`.
    template <std::size_t I>
    struct is_variant_direct_ref_bind_pattern<
        ptn::pat::detail::type_alt_pattern<I,
                                                 ptn::pat::detail::binding_pattern>>
        : std::true_type {};

    template <typename Pattern>
    constexpr bool is_variant_direct_ref_bind_pattern_v =
        is_variant_direct_ref_bind_pattern<std::decay_t<Pattern>>::value;

    template <typename Pattern>
    struct guarded_inner_pattern;

    template <typename Inner, typename Pred>
    struct guarded_inner_pattern<ptn::pat::mod::guarded_pattern<Inner, Pred>> {
      using type = Inner;
    };

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

    template <typename Subject,
              typename CasesTuple,
              std::size_t ActiveIndex,
              std::size_t I = 0,
              std::size_t N =
                  std::tuple_size_v<std::remove_reference_t<CasesTuple>>>
    struct variant_simple_case_index_for_alt {
      using tuple_t   = std::remove_reference_t<CasesTuple>;
      using case_t    = std::tuple_element_t<I, tuple_t>;
      using pattern_t = traits::case_pattern_t<case_t>;

      static constexpr std::size_t value =
          simple_variant_pattern_matches_alt_index<pattern_t,
                                                   Subject,
                                                   ActiveIndex>()
              ? I
              : variant_simple_case_index_for_alt<Subject,
                                                  CasesTuple,
                                                  ActiveIndex,
                                                  I + 1,
                                                  N>::value;
    };

    template <typename Subject,
              typename CasesTuple,
              std::size_t ActiveIndex,
              std::size_t N>
    struct variant_simple_case_index_for_alt<Subject,
                                             CasesTuple,
                                             ActiveIndex,
                                             N,
                                             N> {
      static constexpr std::size_t value = N;
    };

    template <typename Subject,
              typename CasesTuple,
              typename CaseIndex,
              std::size_t... AltIndex>
    constexpr auto
    make_variant_simple_case_index_table(std::index_sequence<AltIndex...>) {
      return std::array<CaseIndex, sizeof...(AltIndex)>{
          static_cast<CaseIndex>(
              variant_simple_case_index_for_alt<Subject,
                                                CasesTuple,
                                                AltIndex>::value)...};
    }

    template <std::size_t CaseCount>
    using variant_case_index_t = std::conditional_t<
        (CaseCount <= std::numeric_limits<std::uint8_t>::max()),
        std::uint8_t,
        std::conditional_t<
            (CaseCount <= std::numeric_limits<std::uint16_t>::max()),
            std::uint16_t,
            std::conditional_t<
                (CaseCount <= std::numeric_limits<std::uint32_t>::max()),
                std::uint32_t,
                std::size_t>>>;

    template <
        typename Subject,
        typename CasesTuple,
        typename SubjectValue =
            std::remove_cv_t<std::remove_reference_t<Subject>>,
        std::size_t AltCount = std::variant_size_v<SubjectValue>>
    struct variant_simple_dispatch_metadata {
      static constexpr std::size_t case_count = std::tuple_size_v<
          std::remove_reference_t<CasesTuple>>;
      using case_index_t = variant_case_index_t<case_count>;
      static_assert(
          case_count
              <= static_cast<std::size_t>(
                  std::numeric_limits<case_index_t>::max()),
          "variant simple dispatch case index exceeds compact type range");

      alignas(64) static constexpr auto case_index_table =
          make_variant_simple_case_index_table<Subject,
                                               CasesTuple,
                                               case_index_t>(
              std::make_index_sequence<AltCount>{});
    };

    template <typename Subject,
              typename CasesTuple,
              std::size_t ActiveIndex,
              std::size_t I = 0,
              std::size_t N =
                  std::tuple_size_v<std::remove_reference_t<CasesTuple>>>
    struct variant_typed_case_first_match_index_for_alt {
      using tuple_t   = std::remove_reference_t<CasesTuple>;
      using case_t    = std::tuple_element_t<I, tuple_t>;
      using pattern_t = traits::case_pattern_t<case_t>;

      static constexpr std::size_t value =
          variant_pattern_maybe_matches_alt_index<pattern_t,
                                                  Subject,
                                                  ActiveIndex>()
              ? I
              : variant_typed_case_first_match_index_for_alt<Subject,
                                                             CasesTuple,
                                                             ActiveIndex,
                                                             I + 1,
                                                             N>::value;
    };

    template <typename Subject,
              typename CasesTuple,
              std::size_t ActiveIndex,
              std::size_t N>
    struct variant_typed_case_first_match_index_for_alt<Subject,
                                                        CasesTuple,
                                                        ActiveIndex,
                                                        N,
                                                        N> {
      static constexpr std::size_t value = N;
    };

    template <typename Subject,
              typename CasesTuple,
              std::size_t ActiveIndex,
              std::size_t I = 0,
              std::size_t N =
                  std::tuple_size_v<std::remove_reference_t<CasesTuple>>>
    struct variant_typed_case_last_match_index_exclusive_for_alt {
      using tuple_t   = std::remove_reference_t<CasesTuple>;
      using case_t    = std::tuple_element_t<I, tuple_t>;
      using pattern_t = traits::case_pattern_t<case_t>;

      static constexpr std::size_t tail =
          variant_typed_case_last_match_index_exclusive_for_alt<Subject,
                                                                CasesTuple,
                                                                ActiveIndex,
                                                                I + 1,
                                                                N>::value;

      static constexpr std::size_t value =
          variant_pattern_maybe_matches_alt_index<pattern_t,
                                                  Subject,
                                                  ActiveIndex>()
              ? (tail > (I + 1) ? tail : (I + 1))
              : tail;
    };

    template <typename Subject,
              typename CasesTuple,
              std::size_t ActiveIndex,
              std::size_t N>
    struct variant_typed_case_last_match_index_exclusive_for_alt<Subject,
                                                                  CasesTuple,
                                                                  ActiveIndex,
                                                                  N,
                                                                  N> {
      static constexpr std::size_t value = 0;
    };

    template <typename Subject,
              typename CasesTuple,
              std::size_t ActiveIndex,
              std::size_t CaseCount =
                  std::tuple_size_v<std::remove_reference_t<CasesTuple>>>
    struct variant_typed_case_range_for_alt {
      static constexpr std::size_t begin =
          variant_typed_case_first_match_index_for_alt<Subject,
                                                       CasesTuple,
                                                       ActiveIndex>::value;

      static constexpr std::size_t end =
          variant_typed_case_last_match_index_exclusive_for_alt<Subject,
                                                                 CasesTuple,
                                                                 ActiveIndex>::value;

      static constexpr bool has_any = begin < end && begin < CaseCount;
    };

    // Stores the residual case slice for one variant alternative.
    // A bucketed variant plan first routes by `variant.index()`, then replays
    // only this `[begin, end)` slice to preserve first-match semantics.
    struct variant_typed_case_bucket {
      std::size_t begin;
      std::size_t end;
      bool        has_any;
    };

    template <typename Subject,
              typename CasesTuple,
              std::size_t CaseCount,
              std::size_t... AltIndex>
    constexpr auto make_typed_variant_case_bucket_table(
        std::index_sequence<AltIndex...>) {
      using bucket_t = variant_typed_case_bucket;

      // Materializes every per-alt residual slice once.
      // The dispatch plan reuses this table instead of recomputing the same
      // range traits at each dispatch site.
      return std::array<bucket_t, sizeof...(AltIndex)>{
          bucket_t{
              variant_typed_case_range_for_alt<Subject,
                                               CasesTuple,
                                               AltIndex,
                                               CaseCount>::begin,
              variant_typed_case_range_for_alt<Subject,
                                               CasesTuple,
                                               AltIndex,
                                               CaseCount>::end,
              variant_typed_case_range_for_alt<Subject,
                                               CasesTuple,
                                               AltIndex,
                                               CaseCount>::has_any}...};
    }

    template <std::size_t UsedAltCount>
    using variant_compact_alt_index_t = std::conditional_t<
        (UsedAltCount <= std::numeric_limits<std::uint8_t>::max()),
        std::uint8_t,
        std::conditional_t<
            (UsedAltCount <= std::numeric_limits<std::uint16_t>::max()),
            std::uint16_t,
            std::uint32_t>>;

    template <typename BucketTable, std::size_t... AltIndex>
    constexpr std::size_t count_typed_variant_compact_dispatch_alts(
        const BucketTable &bucket_table,
        std::index_sequence<AltIndex...>) {
      // Counts only alternatives that keep a non-empty residual slice.
      // Empty alternatives fall straight to the default path.
      return (std::size_t{0}
              + ...
              + (bucket_table[AltIndex].has_any ? std::size_t{1}
                                                 : std::size_t{0}));
    }

    template <typename CompactIndex,
              std::size_t AltCount,
              typename BucketTable,
              std::size_t... AltIndex>
    constexpr auto make_typed_variant_compact_alt_index_map(
        const BucketTable &bucket_table,
        std::index_sequence<AltIndex...>) {
      std::array<CompactIndex, AltCount> map{};
      constexpr CompactIndex invalid = std::numeric_limits<CompactIndex>::max();

      for (auto &entry : map) {
        entry = invalid;
      }

      CompactIndex next = 0;
      // Assigns dense cold-path slots only to alternatives that still own a
      // residual slice after planning.
      (void) std::initializer_list<int>{
          (bucket_table[AltIndex].has_any
               ? (map[AltIndex] = next++, 0)
               : 0)...};

      return map;
    }

    template <typename CompactIndex, std::size_t AltCount, std::size_t... I>
    constexpr auto make_identity_variant_compact_alt_index_map(
        std::index_sequence<I...>) {
      return std::array<CompactIndex, AltCount>{
          static_cast<CompactIndex>(I)...};
    }

    template <
        typename Subject,
        typename CasesTuple,
        typename SubjectValue =
            std::remove_cv_t<std::remove_reference_t<Subject>>,
        std::size_t AltCount = std::variant_size_v<SubjectValue>>
    struct typed_variant_dispatch_metadata {
      static constexpr std::size_t case_count =
          std::tuple_size_v<std::remove_reference_t<CasesTuple>>;
      using case_bucket_t = variant_typed_case_bucket;

      // Maps each full variant index to the residual case slice that must be
      // replayed after that alternative wins the primary dispatch.
      static constexpr auto case_bucket_table =
          make_typed_variant_case_bucket_table<Subject,
                                               CasesTuple,
                                               case_count>(
              std::make_index_sequence<AltCount>{});

      static constexpr std::size_t used_alt_count =
          count_typed_variant_compact_dispatch_alts(
              case_bucket_table,
              std::make_index_sequence<AltCount>{});

      using compact_index_t = variant_compact_alt_index_t<used_alt_count>;
      static constexpr compact_index_t k_invalid_compact_index =
          std::numeric_limits<compact_index_t>::max();

      // Maps a full variant index to the compact cold-path trampoline table.
      // Alternatives with no residual slice keep the invalid sentinel.
      static constexpr auto compact_alt_index_map =
          make_typed_variant_compact_alt_index_map<compact_index_t,
                                                   AltCount>(
              case_bucket_table,
              std::make_index_sequence<AltCount>{});
    };

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

    template <typename Case, typename Subject>
    struct is_static_literal_dispatch_case {
      using case_t       = std::remove_reference_t<Case>;
      using pattern_t    = traits::case_pattern_t<case_t>;
      using handler_t    = traits::case_handler_t<case_t>;
      using bound_args_t = pat::base::binding_args_t<pattern_t, Subject>;

      static constexpr bool literal_subject_compatible =
          is_static_literal_subject_compatible<
              pattern_t,
              Subject,
              is_static_literal_pattern_v<pattern_t>>::value;

      static constexpr bool value =
          (is_wildcard_pattern_v<pattern_t> || literal_subject_compatible)
          && std::tuple_size_v<bound_args_t> == 0
          && std::is_invocable_v<handler_t>;
    };

    // Describes the primary discriminator a single case contributes to the IR.
    // This is the key the planner may use before any residual checking.
    enum class case_discriminator_kind {
      opaque,
      wildcard,
      runtime_literal,
      static_literal,
      variant_alt
    };

    // Describes the work that still remains after primary-key routing.
    // `guard` means the key is usable, but the bucket must still evaluate a
    // guard. `structural` means the bucket must re-enter general matching.
    enum class case_residual_kind {
      none,
      guard,
      structural
    };

    // Describes how much the case's binding behavior constrains lowering.
    // `general` means the planner must assume full matcher replay semantics.
    enum class case_binding_kind {
      none,
      direct_ref,
      general
    };

    template <typename Pattern,
              bool = is_guarded_pattern_v<std::decay_t<Pattern>>>
    struct unguarded_pattern {
      using type = std::decay_t<Pattern>;
    };

    template <typename Pattern>
    struct unguarded_pattern<Pattern, true> {
      using type = typename guarded_inner_pattern<
          std::decay_t<Pattern>>::type;
    };

    template <typename Pattern>
    using unguarded_pattern_t = typename unguarded_pattern<Pattern>::type;

    // Analyzes one case into lowering-friendly IR.
    // This is the seam between the pattern DSL and the planner: new pattern
    // kinds should first describe their discriminator, residual work, and
    // binding shape here before any concrete dispatch plan is chosen.
    template <typename Case, typename Subject>
    struct case_analysis {
      using case_t          = std::remove_reference_t<Case>;
      using pattern_t       = traits::case_pattern_t<case_t>;
      // Strips a guard wrapper so discriminator analysis sees the underlying
      // pattern shape rather than the guard adapter itself.
      using core_pattern_t  = unguarded_pattern_t<pattern_t>;
      using handler_t       = traits::case_handler_t<case_t>;
      // Captures the values this case would bind into the handler.
      using bound_args_t    = pat::base::binding_args_t<pattern_t, Subject>;
      using subject_t       = std::remove_cv_t<std::remove_reference_t<Subject>>;

      // Guards never block key extraction by themselves, but they always force
      // residual work after the primary dispatch key hits.
      static constexpr bool is_guarded = is_guarded_pattern_v<pattern_t>;
      // Any non-empty binding tuple means this case changes handler inputs.
      static constexpr bool has_bindings =
          std::tuple_size_v<bound_args_t> != 0;
      static constexpr bool is_wildcard =
          is_wildcard_pattern_v<pattern_t>;

      // Selects the primary key this case exposes to the planner.
      // The order matters: wildcard takes precedence over any inner shape,
      // then static literals, then runtime literals, then variant alt keys.
      // Anything else is opaque to the current planner.
      static constexpr case_discriminator_kind discriminator_kind =
          is_wildcard
              ? case_discriminator_kind::wildcard
              : (is_static_literal_pattern_v<core_pattern_t>
                     ? case_discriminator_kind::static_literal
                     : (is_simple_literal_pattern_v<core_pattern_t>
                            ? case_discriminator_kind::runtime_literal
                            : ((is_variant_type_is_pattern_v<core_pattern_t>
                                || is_variant_type_alt_pattern_v<core_pattern_t>)
                                   ? case_discriminator_kind::variant_alt
                                   : case_discriminator_kind::opaque)));

      // Records whether binding can stay on a direct-ref fast path.
      // `direct_ref` is the only binding form the current variant lowering can
      // keep without replaying the full matcher.
      static constexpr case_binding_kind binding_kind =
          !has_bindings
              ? case_binding_kind::none
              : (is_variant_direct_ref_bind_pattern_v<core_pattern_t>
                     ? case_binding_kind::direct_ref
                     : case_binding_kind::general);

      // Records what must still happen after primary-key routing.
      // The residual ordering is also intentional:
      // - A guard always becomes a residual guard check.
      // - A non-simple variant case still has a usable variant-alt key, but
      //   must replay structural matching inside that alt bucket.
      // - Opaque discriminators and general bindings force structural replay.
      // - Everything else is fully handled by primary dispatch alone.
      static constexpr case_residual_kind residual_kind =
          is_guarded
              ? case_residual_kind::guard
              : ((discriminator_kind == case_discriminator_kind::variant_alt
                  && !is_simple_variant_dispatch_case<case_t, Subject>::value)
                     || discriminator_kind == case_discriminator_kind::opaque
                     || binding_kind == case_binding_kind::general
                     ? case_residual_kind::structural
                     : case_residual_kind::none);

      // These flags answer a narrower question than `discriminator_kind`:
      // they say whether the whole case satisfies one concrete plan family.
      static constexpr bool supports_simple_literal_dispatch =
          is_simple_literal_dispatch_case<case_t, Subject>::value;
      static constexpr bool supports_static_literal_dispatch =
          is_static_literal_dispatch_case<case_t, Subject>::value;
      static constexpr bool supports_variant_direct_dispatch =
          is_simple_variant_dispatch_case<case_t, Subject>::value;
    };

    // Lowered dispatch is enabled only when every case qualifies.
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

    template <typename Subject,
              typename CasesTuple,
              std::size_t I = 0,
              std::size_t N =
                  std::tuple_size_v<std::remove_reference_t<CasesTuple>>>
    struct is_static_literal_dispatch_sequence {
      using tuple_t   = std::remove_reference_t<CasesTuple>;
      using case_t    = std::tuple_element_t<I, tuple_t>;
      using pattern_t = traits::case_pattern_t<case_t>;

      // Every case in the sequence must satisfy the static-literal case rules.
      static constexpr bool current_case_ok =
          is_static_literal_dispatch_case<case_t, Subject>::value;

      // A wildcard is accepted only as the final default case.
      static constexpr bool value =
          current_case_ok
          && (is_wildcard_pattern_v<pattern_t>
                  ? (I + 1 == N)
                  : is_static_literal_dispatch_sequence<Subject,
                                                        CasesTuple,
                                                        I + 1,
                                                        N>::value);
    };

    template <typename Subject, typename CasesTuple, std::size_t N>
    struct is_static_literal_dispatch_sequence<Subject,
                                               CasesTuple,
                                               N,
                                               N> : std::true_type {};

    template <typename Subject, typename CasesTuple>
    struct is_static_literal_dispatch_cases_tuple : std::false_type {};

    template <typename Subject, typename... Cases>
    struct is_static_literal_dispatch_cases_tuple<Subject, std::tuple<Cases...>>
        : std::bool_constant<is_static_literal_dispatch_sequence<
              Subject,
              std::tuple<Cases...>>::value> {};

    template <typename Subject, typename CasesTuple>
    constexpr bool is_static_literal_dispatch_cases_tuple_v =
        is_static_literal_dispatch_cases_tuple<
            Subject,
            std::remove_reference_t<CasesTuple>>::value;

    template <typename Subject,
              typename CasesTuple,
              std::size_t I = 0,
              std::size_t N =
                  std::tuple_size_v<std::remove_reference_t<CasesTuple>>>
    struct static_literal_case_count {
      using tuple_t   = std::remove_reference_t<CasesTuple>;
      using case_t    = std::tuple_element_t<I, tuple_t>;
      using pattern_t = traits::case_pattern_t<case_t>;

      // Counts only actual static-literal cases. Wildcards contribute to the
      // default slot, but not to the dense literal table density.
      static constexpr std::size_t value =
          (is_static_literal_pattern_v<pattern_t> ? std::size_t{1}
                                                  : std::size_t{0})
          + static_literal_case_count<Subject, CasesTuple, I + 1, N>::value;
    };

    template <typename Subject, typename CasesTuple, std::size_t N>
    struct static_literal_case_count<Subject, CasesTuple, N, N>
        : std::integral_constant<std::size_t, 0> {};

    template <typename Key>
    constexpr std::size_t static_literal_range_width(Key min_value,
                                                     Key max_value) {
      // Computes the closed interval width `[min_value, max_value]`.
      if constexpr (std::is_signed_v<Key>) {
        return static_cast<std::size_t>(
                   static_cast<std::intmax_t>(max_value)
                   - static_cast<std::intmax_t>(min_value))
               + 1u;
      }
      else {
        return static_cast<std::size_t>(
                   static_cast<std::uintmax_t>(max_value)
                   - static_cast<std::uintmax_t>(min_value))
               + 1u;
      }
    }

    template <typename Subject,
              typename CasesTuple,
              std::size_t I = 0,
              std::size_t N =
                  std::tuple_size_v<std::remove_reference_t<CasesTuple>>>
    struct static_literal_value_range;

    template <typename Subject,
              typename CasesTuple,
              std::size_t I,
              std::size_t N,
              bool IsLiteral>
    struct static_literal_value_range_case;

    template <typename Subject,
              typename CasesTuple,
              std::size_t I,
              std::size_t N>
    struct static_literal_value_range_case<Subject,
                                           CasesTuple,
                                           I,
                                           N,
                                           true> {
      using tuple_t   = std::remove_reference_t<CasesTuple>;
      using subject_t = std::remove_cv_t<std::remove_reference_t<Subject>>;
      using key_t     = literal_subject_key_t<subject_t>;
      using case_t    = std::tuple_element_t<I, tuple_t>;
      using pattern_t = traits::case_pattern_t<case_t>;
      using tail_t = static_literal_value_range<Subject,
                                                CasesTuple,
                                                I + 1,
                                                N>;

      // Folds the current static literal into the tail min/max summary.
      static constexpr key_t current_value =
          static_literal_case_value<pattern_t, Subject>::value;
      static constexpr bool has_any = true;

      static constexpr key_t min =
          tail_t::has_any
              ? (current_value < tail_t::min ? current_value : tail_t::min)
              : current_value;

      static constexpr key_t max =
          tail_t::has_any
              ? (current_value > tail_t::max ? current_value : tail_t::max)
              : current_value;
    };

    template <typename Subject,
              typename CasesTuple,
              std::size_t I,
              std::size_t N>
    struct static_literal_value_range_case<Subject,
                                           CasesTuple,
                                           I,
                                           N,
                                           false> {
      using tail_t = static_literal_value_range<Subject,
                                                CasesTuple,
                                                I + 1,
                                                N>;
      using key_t = typename tail_t::key_t;

      // Non-literal cases do not affect the dense-table key range.
      static constexpr bool has_any = tail_t::has_any;
      static constexpr key_t min    = tail_t::min;
      static constexpr key_t max    = tail_t::max;
    };

    template <typename Subject,
              typename CasesTuple,
              std::size_t I,
              std::size_t N>
    struct static_literal_value_range
        : static_literal_value_range_case<
              Subject,
              CasesTuple,
              I,
              N,
              is_static_literal_pattern_v<traits::case_pattern_t<
                  std::tuple_element_t<I,
                                       std::remove_reference_t<CasesTuple>>>>> {
    };

    template <typename Subject, typename CasesTuple, std::size_t N>
    struct static_literal_value_range<Subject, CasesTuple, N, N> {
      using subject_t = std::remove_cv_t<std::remove_reference_t<Subject>>;
      using key_t     = literal_subject_key_t<subject_t>;

      static constexpr bool has_any = false;
      static constexpr key_t min    = key_t{};
      static constexpr key_t max    = key_t{};
    };

    template <typename Tuple, std::size_t N>
    struct tuple_last_case_is_wildcard
        : std::bool_constant<is_wildcard_pattern_v<traits::case_pattern_t<
              std::tuple_element_t<N - 1, Tuple>>>> {};

    template <typename Tuple>
    struct tuple_last_case_is_wildcard<Tuple, 0> : std::false_type {};

    template <typename Subject,
              typename CasesTuple,
              typename SubjectValue =
                  std::remove_cv_t<std::remove_reference_t<Subject>>>
    struct static_literal_dispatch_metadata {
      using tuple_t = std::remove_reference_t<CasesTuple>;
      using key_t   = literal_subject_key_t<SubjectValue>;

      // `case_count` includes the trailing wildcard default if present.
      static constexpr std::size_t case_count =
          std::tuple_size_v<tuple_t>;
      // `literal_case_count` counts only keyed literal entries.
      static constexpr std::size_t literal_case_count =
          static_literal_case_count<Subject, CasesTuple>::value;
      using case_index_t = variant_case_index_t<case_count>;

      using value_range_t =
          static_literal_value_range<Subject, CasesTuple>;

      // These values describe the dense candidate key span.
      static constexpr bool has_literal_cases = value_range_t::has_any;
      static constexpr key_t min_value        = value_range_t::min;
      static constexpr key_t max_value        = value_range_t::max;
      static constexpr std::size_t range_size =
          has_literal_cases
              ? static_literal_range_width(min_value, max_value)
              : 0u;

      static constexpr case_index_t k_invalid_case_index =
          std::numeric_limits<case_index_t>::max();

      // The final wildcard, when present, is reused as the dense dispatch
      // default slot.
      static constexpr bool has_default_case =
          tuple_last_case_is_wildcard<tuple_t, case_count>::value;

      static constexpr case_index_t default_case_index =
          has_default_case ? static_cast<case_index_t>(case_count - 1)
                           : k_invalid_case_index;

      // Dense dispatch is worthwhile only when the key span stays small enough
      // and dense enough relative to the number of literal cases.
      static constexpr bool use_dense_table =
          has_literal_cases
          && literal_case_count > 0
          && case_count <= k_static_literal_dense_dispatch_max_cases
          && range_size <= k_static_literal_dense_dispatch_max_span
          && range_size
                 <= literal_case_count
                        * k_static_literal_dense_dispatch_max_density;
    };

    template <typename Subject, typename CasesTuple, bool Eligible>
    struct is_static_literal_dense_dispatch_enabled : std::false_type {};

    template <typename Subject, typename CasesTuple>
    struct is_static_literal_dense_dispatch_enabled<Subject, CasesTuple, true>
        : std::bool_constant<
              static_literal_dispatch_metadata<Subject, CasesTuple>::
                  use_dense_table> {};

    // Summarizes the three lowering rules discussed for the engine.
    // `full` means direct keyed dispatch is legal, `bucketed` means keyed
    // dispatch may narrow the search before replay, and `none` means the
    // planner must fall back to sequential evaluation.
    enum class lowering_legality {
      none,
      bucketed,
      full
    };

    // Names the concrete runtime shape selected after legality analysis.
    // Multiple plan kinds may share the same legality grade.
    enum class dispatch_plan_kind {
      sequential,
      literal_linear,
      static_literal_dense,
      variant_simple,
      variant_alt_bucketed
    };

    template <typename Subject, typename CasesTuple>
    struct case_sequence_ir;

    template <typename Subject, typename... Cases>
    struct case_sequence_ir<Subject, std::tuple<Cases...>> {
      using subject_t  = std::remove_cv_t<std::remove_reference_t<Subject>>;
      // Retains per-case IR so future planners can inspect the whole sequence
      // without re-running case-level trait analysis.
      using analyses_t = std::tuple<case_analysis<Cases, Subject>...>;

      static constexpr std::size_t case_count = sizeof...(Cases);

      // `true` if any case still needs work after primary-key routing.
      static constexpr bool has_residual_cases =
          ((case_analysis<Cases, Subject>::residual_kind
            != case_residual_kind::none)
           || ... || false);

      // `true` if any case binds in a way that blocks the most aggressive
      // lowering shapes.
      static constexpr bool has_binding_constraints =
          ((case_analysis<Cases, Subject>::binding_kind
            != case_binding_kind::none)
           || ... || false);

      // `true` if at least one case exposes no planner-visible discriminator.
      static constexpr bool has_opaque_discriminator =
          ((case_analysis<Cases, Subject>::discriminator_kind
            == case_discriminator_kind::opaque)
           || ... || false);

      // Rule 1: every case participates in dense static-literal dispatch.
      // This is stronger than "all cases are static literals": the resulting
      // literal span must also satisfy the dense-table heuristics.
      static constexpr bool can_use_static_literal_dispatch =
          is_static_literal_dense_dispatch_enabled<
              subject_t,
              std::tuple<Cases...>,
              ((std::is_integral_v<subject_t> || std::is_enum_v<subject_t>)
               && (case_analysis<Cases, Subject>::
                       supports_static_literal_dispatch
                   && ...))>::value;

      // Rule 1: every case participates in direct runtime literal dispatch.
      // This is the older linear literal lowering. It stays available when
      // dense static-literal dispatch is not legal or not profitable.
      static constexpr bool can_use_simple_literal_dispatch =
          (std::is_integral_v<subject_t> || std::is_enum_v<subject_t>)
          && (case_analysis<Cases, Subject>::supports_simple_literal_dispatch
              && ...);

      // Rule 1: every case can dispatch directly from the active variant alt.
      // No residual replay is needed once the active alt is known.
      static constexpr bool can_use_simple_variant_dispatch =
          is_variant_subject_v<subject_t>
          && (case_analysis<Cases, Subject>::supports_variant_direct_dispatch
              && ...);

      // Rule 2: the variant alt is still a usable primary key even if some
      // cases need residual replay within the selected bucket.
      // Today this is the only bucketed lowering family. The planner will
      // still reject non-variant subjects here.
      static constexpr bool can_use_variant_alt_dispatch =
          is_variant_subject_v<subject_t>;

      // Collapses the aggregate IR back to the three lowering rules.
      // Precedence matters:
      // - `full` wins whenever a direct literal or direct variant plan exists.
      // - Otherwise a variant subject may still use `bucketed`.
      // - Everything else falls back to sequential evaluation.
      static constexpr lowering_legality legality =
          can_use_static_literal_dispatch || can_use_simple_literal_dispatch
                  || can_use_simple_variant_dispatch
              ? lowering_legality::full
              : (can_use_variant_alt_dispatch ? lowering_legality::bucketed
                                              : lowering_legality::none);
    };

    // Wraps the sequence IR in the planner-facing analysis API.
    // This stays declarative so future planners can reuse the same legality
    // checks without depending on evaluator internals.
    template <typename Subject, typename CasesTuple>
    struct lowering_analysis {
      using ir_t = case_sequence_ir<Subject,
                                    std::remove_reference_t<CasesTuple>>;

      // Re-exports the sequence IR in the shape expected by plan selection.
      static constexpr bool can_use_static_literal_dispatch =
          ir_t::can_use_static_literal_dispatch;
      static constexpr bool can_use_simple_literal_dispatch =
          ir_t::can_use_simple_literal_dispatch;
      static constexpr bool can_use_simple_variant_dispatch =
          ir_t::can_use_simple_variant_dispatch;
      static constexpr bool can_use_variant_alt_dispatch =
          ir_t::can_use_variant_alt_dispatch;
      static constexpr lowering_legality legality = ir_t::legality;
    };

    template <typename Subject, typename CasesTuple>
    struct sequential_dispatch_plan {
      using subject_t     = std::remove_cv_t<std::remove_reference_t<Subject>>;
      using cases_tuple_t = std::remove_reference_t<CasesTuple>;

      static constexpr dispatch_plan_kind kind =
          dispatch_plan_kind::sequential;
      // Rule 3: no keyed lowering is legal, so evaluator replays all cases.
      static constexpr lowering_legality legality =
          lowering_legality::none;
    };

    template <typename Subject, typename CasesTuple>
    struct literal_linear_dispatch_plan {
      using subject_t     = std::remove_cv_t<std::remove_reference_t<Subject>>;
      using cases_tuple_t = std::remove_reference_t<CasesTuple>;

      static constexpr dispatch_plan_kind kind =
          dispatch_plan_kind::literal_linear;
      // Rule 1: each case is already a direct literal dispatch case.
      static constexpr lowering_legality legality =
          lowering_legality::full;
    };

    template <typename Subject,
              typename CasesTuple,
              typename SubjectValue =
                  std::remove_cv_t<std::remove_reference_t<Subject>>>
    struct static_literal_dense_dispatch_plan {
      using subject_t     = SubjectValue;
      using cases_tuple_t = std::remove_reference_t<CasesTuple>;
      using metadata_t =
          static_literal_dispatch_metadata<Subject, CasesTuple, SubjectValue>;
      using key_t        = typename metadata_t::key_t;
      using case_index_t = typename metadata_t::case_index_t;

      static constexpr dispatch_plan_kind kind =
          dispatch_plan_kind::static_literal_dense;
      static constexpr lowering_legality legality =
          lowering_legality::full;

      // Re-exports the dense-table metadata the evaluator needs at runtime.
      static constexpr std::size_t case_count =
          metadata_t::case_count;
      static constexpr std::size_t literal_case_count =
          metadata_t::literal_case_count;
      static constexpr std::size_t range_size =
          metadata_t::range_size;
      static constexpr bool has_default_case =
          metadata_t::has_default_case;
      static constexpr key_t min_value = metadata_t::min_value;
      static constexpr key_t max_value = metadata_t::max_value;
      static constexpr case_index_t default_case_index =
          metadata_t::default_case_index;
      static constexpr case_index_t k_invalid_case_index =
          metadata_t::k_invalid_case_index;
    };

    template <typename Subject,
              typename CasesTuple,
              typename SubjectValue =
                  std::remove_cv_t<std::remove_reference_t<Subject>>,
              std::size_t AltCount = std::variant_size_v<SubjectValue>>
    struct variant_simple_dispatch_plan {
      using subject_t     = SubjectValue;
      using cases_tuple_t = std::remove_reference_t<CasesTuple>;
      using metadata_t = variant_simple_dispatch_metadata<Subject,
                                                          CasesTuple,
                                                          SubjectValue,
                                                          AltCount>;
      using case_index_t = typename metadata_t::case_index_t;
      using compact_index_t = variant_compact_alt_index_t<AltCount>;

      static constexpr dispatch_plan_kind kind =
          dispatch_plan_kind::variant_simple;
      static constexpr lowering_legality legality =
          lowering_legality::full;

      // All variant alternatives stay addressable in the simple plan.
      static constexpr std::size_t alt_count      = AltCount;
      static constexpr std::size_t case_count     = metadata_t::case_count;
      static constexpr std::size_t used_alt_count = AltCount;
      static constexpr variant_dispatch_tier tier =
          variant_dispatch_tier_for_alt_count<AltCount>();
      // Maps each alternative directly to the terminal case chosen for it.
      static constexpr auto case_index_table =
          metadata_t::case_index_table;
      // The simple plan keeps every alternative, so the compact map is the
      // identity function.
      static constexpr auto compact_alt_index_map =
          make_identity_variant_compact_alt_index_map<compact_index_t,
                                                      AltCount>(
              std::make_index_sequence<AltCount>{});
      static constexpr compact_index_t k_invalid_compact_index =
          std::numeric_limits<compact_index_t>::max();

      template <std::size_t AltIndex>
      struct case_entry {
        static_assert(AltIndex < AltCount, "variant alt index out of range");

        // `case_index` names the terminal case that handles this alternative.
        static constexpr case_index_t case_index =
            case_index_table[AltIndex];
        // `false` means this alternative falls through to `otherwise`.
        static constexpr bool has_any = case_index < case_count;
      };
    };

    template <typename Subject,
              typename CasesTuple,
              typename SubjectValue =
                  std::remove_cv_t<std::remove_reference_t<Subject>>,
              std::size_t AltCount = std::variant_size_v<SubjectValue>>
    struct variant_alt_bucketed_dispatch_plan {
      using subject_t     = SubjectValue;
      using cases_tuple_t = std::remove_reference_t<CasesTuple>;
      using metadata_t = typed_variant_dispatch_metadata<Subject,
                                                         CasesTuple,
                                                         SubjectValue,
                                                         AltCount>;
      using compact_index_t = typename metadata_t::compact_index_t;

      static constexpr dispatch_plan_kind kind =
          dispatch_plan_kind::variant_alt_bucketed;
      static constexpr lowering_legality legality =
          lowering_legality::bucketed;

      // Only alternatives with a non-empty residual slice stay in the compact
      // cold-path table.
      static constexpr std::size_t alt_count      = AltCount;
      static constexpr std::size_t case_count     = metadata_t::case_count;
      static constexpr std::size_t used_alt_count = metadata_t::used_alt_count;
      static constexpr variant_dispatch_tier tier =
          variant_dispatch_tier_for_alt_count<AltCount>();
      // The evaluator uses this map only on the cold path.
      static constexpr auto compact_alt_index_map =
          metadata_t::compact_alt_index_map;
      // This table is the real payload of bucketed lowering: each alternative
      // maps to the `[begin, end)` slice that must be replayed.
      static constexpr auto case_bucket_table =
          metadata_t::case_bucket_table;
      static constexpr compact_index_t k_invalid_compact_index =
          metadata_t::k_invalid_compact_index;

      template <std::size_t AltIndex>
      struct case_bucket {
        static_assert(AltIndex < AltCount, "variant alt index out of range");

        // Type-level view for a single precomputed residual case bucket.
        // The evaluator replays exactly this half-open interval on a hit.
        static constexpr std::size_t begin =
            case_bucket_table[AltIndex].begin;
        static constexpr std::size_t end =
            case_bucket_table[AltIndex].end;
        static constexpr bool has_any =
            case_bucket_table[AltIndex].has_any;
      };
    };

    template <typename Subject,
              typename CasesTuple,
              dispatch_plan_kind Kind>
    struct dispatch_plan_for_kind;

    template <typename Subject, typename CasesTuple>
    struct dispatch_plan_for_kind<Subject,
                                  CasesTuple,
                                  dispatch_plan_kind::sequential> {
      using type = sequential_dispatch_plan<Subject, CasesTuple>;
    };

    template <typename Subject, typename CasesTuple>
    struct dispatch_plan_for_kind<Subject,
                                  CasesTuple,
                                  dispatch_plan_kind::literal_linear> {
      using type = literal_linear_dispatch_plan<Subject, CasesTuple>;
    };

    template <typename Subject, typename CasesTuple>
    struct dispatch_plan_for_kind<Subject,
                                  CasesTuple,
                                  dispatch_plan_kind::static_literal_dense> {
      using type = static_literal_dense_dispatch_plan<Subject, CasesTuple>;
    };

    template <typename Subject, typename CasesTuple>
    struct dispatch_plan_for_kind<Subject,
                                  CasesTuple,
                                  dispatch_plan_kind::variant_simple> {
      using type = variant_simple_dispatch_plan<Subject, CasesTuple>;
    };

    template <typename Subject, typename CasesTuple>
    struct dispatch_plan_for_kind<Subject,
                                  CasesTuple,
                                  dispatch_plan_kind::variant_alt_bucketed> {
      using type =
          variant_alt_bucketed_dispatch_plan<Subject, CasesTuple>;
    };

    // Planned dispatch shape for a case sequence.
    // Evaluators consume the selected plan and its metadata.
    template <typename Subject, typename CasesTuple>
    struct dispatch_plan_selector {
      using analysis_t = lowering_analysis<Subject, CasesTuple>;

      static constexpr dispatch_plan_kind kind =
          analysis_t::can_use_static_literal_dispatch
              ? dispatch_plan_kind::static_literal_dense
              : (analysis_t::can_use_simple_literal_dispatch
                     ? dispatch_plan_kind::literal_linear
                     : (analysis_t::can_use_simple_variant_dispatch
                            ? dispatch_plan_kind::variant_simple
                            : (analysis_t::can_use_variant_alt_dispatch
                                   ? dispatch_plan_kind::variant_alt_bucketed
                                   : dispatch_plan_kind::sequential)));

      using type = typename dispatch_plan_for_kind<Subject,
                                                   CasesTuple,
                                                   kind>::type;
    };

    template <typename Subject, typename CasesTuple>
    using dispatch_plan =
        typename dispatch_plan_selector<Subject, CasesTuple>::type;

  } // namespace detail

  template <typename Subject, typename CasesTuple>
  using dispatch_plan_t = detail::dispatch_plan<Subject, CasesTuple>;

} // namespace ptn::core::common
