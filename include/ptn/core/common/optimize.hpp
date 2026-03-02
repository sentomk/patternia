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

    // Matches `type::is<T>(bind())`.
    template <typename Pattern>
    struct is_variant_direct_ref_bind_pattern : std::false_type {};

    template <typename Alt>
    struct is_variant_direct_ref_bind_pattern<
        ptn::pat::type::detail::type_is_pattern<Alt,
                                                ptn::pat::detail::binding_pattern>>
        : std::true_type {};

    // Matches `type::alt<I>(bind())`.
    template <std::size_t I>
    struct is_variant_direct_ref_bind_pattern<
        ptn::pat::type::detail::type_alt_pattern<I,
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

    template <std::size_t UsedAltCount>
    using variant_compact_alt_index_t = std::conditional_t<
        (UsedAltCount <= std::numeric_limits<std::uint8_t>::max()),
        std::uint8_t,
        std::conditional_t<
            (UsedAltCount <= std::numeric_limits<std::uint16_t>::max()),
            std::uint16_t,
            std::uint32_t>>;

    template <typename Subject,
              typename CasesTuple,
              std::size_t CaseCount,
              std::size_t... AltIndex>
    constexpr std::size_t count_typed_variant_compact_dispatch_alts(
        std::index_sequence<AltIndex...>) {
      return (std::size_t{0}
              + ... +
              (variant_typed_case_range_for_alt<Subject,
                                                CasesTuple,
                                                AltIndex,
                                                CaseCount>::has_any
                   ? std::size_t{1}
                   : std::size_t{0}));
    }

    template <typename CompactIndex,
              typename Subject,
              typename CasesTuple,
              std::size_t AltCount,
              std::size_t CaseCount,
              std::size_t... AltIndex>
    constexpr auto make_typed_variant_compact_alt_index_map(
        std::index_sequence<AltIndex...>) {
      std::array<CompactIndex, AltCount> map{};
      constexpr CompactIndex invalid = std::numeric_limits<CompactIndex>::max();

      for (auto &entry : map) {
        entry = invalid;
      }

      CompactIndex next = 0;
      (void) std::initializer_list<int>{
          (variant_typed_case_range_for_alt<Subject,
                                            CasesTuple,
                                            AltIndex,
                                            CaseCount>::has_any
               ? (map[AltIndex] = next++, 0)
               : 0)...};

      return map;
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

      static constexpr std::size_t used_alt_count =
          count_typed_variant_compact_dispatch_alts<Subject,
                                                    CasesTuple,
                                                    case_count>(
              std::make_index_sequence<AltCount>{});

      using compact_index_t = variant_compact_alt_index_t<used_alt_count>;
      static constexpr compact_index_t k_invalid_compact_index =
          std::numeric_limits<compact_index_t>::max();

      // Flattened index map: variant index -> compact dispatch slot.
      static constexpr auto compact_alt_index_map =
          make_typed_variant_compact_alt_index_map<compact_index_t,
                                                   Subject,
                                                   CasesTuple,
                                                   AltCount,
                                                   case_count>(
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

      static constexpr bool current_case_ok =
          is_static_literal_dispatch_case<case_t, Subject>::value;

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

      static constexpr std::size_t case_count =
          std::tuple_size_v<tuple_t>;
      static constexpr std::size_t literal_case_count =
          static_literal_case_count<Subject, CasesTuple>::value;
      using case_index_t = variant_case_index_t<case_count>;

      using value_range_t =
          static_literal_value_range<Subject, CasesTuple>;

      static constexpr bool has_literal_cases = value_range_t::has_any;
      static constexpr key_t min_value        = value_range_t::min;
      static constexpr key_t max_value        = value_range_t::max;
      static constexpr std::size_t range_size =
          has_literal_cases
              ? static_literal_range_width(min_value, max_value)
              : 0u;

      static constexpr case_index_t k_invalid_case_index =
          std::numeric_limits<case_index_t>::max();

      static constexpr bool has_default_case =
          tuple_last_case_is_wildcard<tuple_t, case_count>::value;

      static constexpr case_index_t default_case_index =
          has_default_case ? static_cast<case_index_t>(case_count - 1)
                           : k_invalid_case_index;

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

    // Semantic lowering grade selected by the planner.
    enum class lowering_legality {
      none,
      bucketed,
      full
    };

    // Concrete dispatch shape executed by the evaluator.
    enum class dispatch_plan_kind {
      sequential,
      literal_linear,
      static_literal_dense,
      variant_simple,
      variant_alt_bucketed
    };

    // Analysis phase for case-sequence lowering.
    // This stays purely declarative so future planners can reuse the same
    // legality checks without depending on evaluator internals.
    template <typename Subject, typename CasesTuple>
    struct lowering_analysis {
      using subject_t = std::remove_cv_t<std::remove_reference_t<Subject>>;

      static constexpr bool can_use_static_literal_dispatch =
          (std::is_integral_v<subject_t> || std::is_enum_v<subject_t>)
          && is_static_literal_dense_dispatch_enabled<
              subject_t,
              CasesTuple,
              is_static_literal_dispatch_cases_tuple_v<subject_t,
                                                       CasesTuple>>::value;

      static constexpr bool can_use_simple_literal_dispatch =
          (std::is_integral_v<subject_t> || std::is_enum_v<subject_t>)
          && is_simple_literal_dispatch_cases_tuple_v<subject_t, CasesTuple>;

      static constexpr bool can_use_simple_variant_dispatch =
          is_variant_subject_v<subject_t>
          && is_simple_variant_dispatch_cases_tuple_v<subject_t, CasesTuple>;

      static constexpr bool can_use_variant_alt_dispatch =
          is_variant_subject_v<subject_t>;

      static constexpr lowering_legality legality =
          can_use_static_literal_dispatch || can_use_simple_variant_dispatch
              ? lowering_legality::full
              : (can_use_variant_alt_dispatch ? lowering_legality::bucketed
                                              : lowering_legality::none);
    };

    // Planned dispatch shape for a case sequence.
    // Evaluators should consume this instead of re-deriving dispatch choices.
    template <typename Subject, typename CasesTuple>
    struct dispatch_plan {
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

      static constexpr lowering_legality legality =
          analysis_t::legality;
    };

  } // namespace detail

  template <typename Subject, typename CasesTuple>
  using dispatch_plan_t = detail::dispatch_plan<Subject, CasesTuple>;

} // namespace ptn::core::common
