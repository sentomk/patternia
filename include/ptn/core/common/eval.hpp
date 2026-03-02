#pragma once

// Common evaluation logic for matching and handler invocation.
//
// This header provides the core runtime evaluation machinery for
// pattern matching, including case matching logic, handler
// invocation with bound values, and sequential case evaluation.

#include <tuple>
#include <type_traits>
#include <utility>
#include <array>
#include <functional>
#include <cstdlib>
#include <initializer_list>

#include "ptn/core/common/common_traits.hpp"
#include "ptn/core/common/diagnostics.hpp"
#include "ptn/core/common/optimize.hpp"

namespace ptn::core::common {
#if defined(_MSC_VER)
#define PTN_DETAIL_NOINLINE __declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
#define PTN_DETAIL_NOINLINE __attribute__((noinline))
#else
#define PTN_DETAIL_NOINLINE
#endif

#define PTN_DETAIL_STATIC_LITERAL_SWITCH_CASE(n)                    \
  case n:                                                           \
    return invoke_static_literal_case_entry<n, Result>(cases)

#define PTN_DETAIL_STATIC_LITERAL_SWITCH_BLOCK_16(base)             \
  PTN_DETAIL_STATIC_LITERAL_SWITCH_CASE((base) + 0);                \
  PTN_DETAIL_STATIC_LITERAL_SWITCH_CASE((base) + 1);                \
  PTN_DETAIL_STATIC_LITERAL_SWITCH_CASE((base) + 2);                \
  PTN_DETAIL_STATIC_LITERAL_SWITCH_CASE((base) + 3);                \
  PTN_DETAIL_STATIC_LITERAL_SWITCH_CASE((base) + 4);                \
  PTN_DETAIL_STATIC_LITERAL_SWITCH_CASE((base) + 5);                \
  PTN_DETAIL_STATIC_LITERAL_SWITCH_CASE((base) + 6);                \
  PTN_DETAIL_STATIC_LITERAL_SWITCH_CASE((base) + 7);                \
  PTN_DETAIL_STATIC_LITERAL_SWITCH_CASE((base) + 8);                \
  PTN_DETAIL_STATIC_LITERAL_SWITCH_CASE((base) + 9);                \
  PTN_DETAIL_STATIC_LITERAL_SWITCH_CASE((base) + 10);               \
  PTN_DETAIL_STATIC_LITERAL_SWITCH_CASE((base) + 11);               \
  PTN_DETAIL_STATIC_LITERAL_SWITCH_CASE((base) + 12);               \
  PTN_DETAIL_STATIC_LITERAL_SWITCH_CASE((base) + 13);               \
  PTN_DETAIL_STATIC_LITERAL_SWITCH_CASE((base) + 14);               \
  PTN_DETAIL_STATIC_LITERAL_SWITCH_CASE((base) + 15)

  // SFINAE Helper for C++17
  namespace detail {
    // Helper type to provide void type for SFINAE expressions
    template <typename... Ts>
    struct void_type {
      using type = void;
    };

    // Detects if a pattern has a .match(subject) member function
    template <typename P, typename S, typename = void>
    struct has_match_member : std::false_type {};

    template <typename P, typename S>
    struct has_match_member<
        P,
        S,
        typename void_type<decltype(std::declval<P>().match(
            std::declval<S>()))>::type> : std::true_type {};

    template <typename P, typename S>
    constexpr bool
        has_match_member_v = has_match_member<P, S>::value;

    // Detects if a pattern has a .bind(subject) member function
    template <typename P, typename S, typename = void>
    struct has_bind_member : std::false_type {};

    template <typename P, typename S>
    struct has_bind_member<
        P,
        S,
        typename void_type<decltype(std::declval<P>().bind(
            std::declval<S>()))>::type> : std::true_type {};

    template <typename P, typename S>
    constexpr bool has_bind_member_v = has_bind_member<P, S>::value;
  } // namespace detail

  // Case matching.

  // Checks if a case's pattern matches a subject value
  // Supports both class-based patterns with .match() and functional
  // patterns
  template <typename Case, typename Subject>
  struct case_matcher {
    using case_type    = Case;
    using subject_type = Subject;
    using pattern_type = traits::case_pattern_t<Case>;

    // Returns true if the pattern matches the subject
    static constexpr bool matches(const case_type    &c,
                                  const subject_type &subject) {
      if constexpr (detail::has_match_member_v<pattern_type,
                                               subject_type>) {
        return c.pattern.match(subject);
      }
      else {
        return static_cast<bool>(std::invoke(c.pattern, subject));
      }
    }
  };

  // Handler invocation logic.

  namespace detail {

    // MSVC-friendly tuple invocation
    // Avoids: std::apply + dependent lambda + if constexpr
    template <typename F, typename Tuple, std::size_t... I>
    constexpr decltype(auto) invoke_from_tuple_impl(
        F &&f, Tuple &&t, std::index_sequence<I...>) {

      using result_t = std::invoke_result_t<
          F,
          decltype(std::get<I>(std::forward<Tuple>(t)))...>;

      if constexpr (std::is_void_v<result_t>) {
        std::invoke(std::forward<F>(f),
                    std::get<I>(std::forward<Tuple>(t))...);
        return;
      }
      else {
        return std::invoke(std::forward<F>(f),
                           std::get<I>(std::forward<Tuple>(t))...);
      }
    }

    template <typename F, typename Tuple>
    constexpr decltype(auto) invoke_from_tuple(F &&f, Tuple &&t) {
      using tuple_t = std::remove_reference_t<Tuple>;
      return invoke_from_tuple_impl(
          std::forward<F>(f),
          std::forward<Tuple>(t),
          std::make_index_sequence<std::tuple_size_v<tuple_t>>{});
    }

  } // namespace detail

  // Invokes a handler with the values bound by its pattern
  //
  // Design A: handler receives only the bound values from pattern
  // matching
  //   handler( pattern.bind(subject)... )
  //   The subject itself is not passed as an extra first argument
  template <typename Case, typename Subject>
  constexpr decltype(auto) invoke_handler(const Case &c,
                                          Subject    &subject) {
    using pattern_type = traits::case_pattern_t<Case>;

    ptn::core::common::static_assert_pattern_has_bind<
        detail::has_bind_member_v<pattern_type, Subject &>>();

    // Extract bound values from the pattern
    auto &&bound_values = c.pattern.bind(subject);
    return detail::invoke_from_tuple(c.handler, bound_values);
  }

  // Case-sequence evaluation.
  namespace detail {
    // Recursive implementation for evaluating a tuple of cases in
    // sequence
    template <std::size_t I,
              typename Subject,
              typename CasesTuple,
              typename Otherwise,
              std::size_t N = std::tuple_size_v<
                  std::remove_reference_t<CasesTuple>>>
    constexpr decltype(auto)
    eval_cases_impl(Subject    &subject,
                    CasesTuple &cases,
                    Otherwise &&otherwise_handler) {
      if constexpr (I >= N) {
        // Base case: no case matched, invoke the otherwise handler
        if constexpr (std::is_invocable_v<Otherwise, Subject &>) {
          return std::forward<Otherwise>(otherwise_handler)(subject);
        }
        else {
          return std::forward<Otherwise>(otherwise_handler)();
        }
      }
      else {
        auto &current_case = std::get<I>(cases);
        using case_t       = std::remove_reference_t<
                  decltype(current_case)>;

        if (case_matcher<case_t, Subject>::matches(current_case,
                                                   subject)) {
          // Found a match, invoke its handler
          return invoke_handler(current_case, subject);
        }
        else {
          // Try the next case.
          return eval_cases_impl<I + 1>(
              subject,
              cases,
              std::forward<Otherwise>(otherwise_handler));
        }
      }
    }
  } // namespace detail

  // Public entry point to evaluate a sequence of cases against a
  // subject Tries each case in order, invoking the first matching
  // handler
  template <typename Subject,
            typename CasesTuple,
            typename Otherwise>
  constexpr decltype(auto)
  eval_cases(Subject    &subject,
             CasesTuple &cases,
             Otherwise &&otherwise_handler) {
    using tuple_t           = std::remove_reference_t<CasesTuple>;
    constexpr std::size_t N = std::tuple_size_v<tuple_t>;

    (void) N; // silence unused warning if needed

    return detail::eval_cases_impl<0>(
        subject, cases, std::forward<Otherwise>(otherwise_handler));
  }

  // Strongly-typed evaluation entry point.
  // This overload forces the match evaluation to return a single,
  // precomputed result type. This is required for expression-style
  // matches where different branches may produce different (but
  // compatible) return types.
  namespace detail {
    template <typename Result>
    [[noreturn]] inline Result unreachable_result() {
      std::abort();
    }

    template <typename Result, typename Subject, typename Otherwise>
    constexpr Result
    invoke_otherwise_typed(Subject    &subject,
                           Otherwise &&otherwise_handler) {
      // Shared typed otherwise flow for both generic and lowered evaluators.
      using otherwise_result = traits::otherwise_result_t<Otherwise,
                                                          Subject>;

      if constexpr (std::is_same_v<otherwise_result,
                                   traits::detail::unreachable_t>) {
        if constexpr (std::is_invocable_v<Otherwise, Subject &>) {
          std::forward<Otherwise>(otherwise_handler)(subject);
        }
        else {
          std::forward<Otherwise>(otherwise_handler)();
        }
        return unreachable_result<Result>();
      }
      else if constexpr (std::is_invocable_v<Otherwise, Subject &>) {
        return static_cast<Result>(
            std::forward<Otherwise>(otherwise_handler)(subject));
      }
      else {
        return static_cast<Result>(
            std::forward<Otherwise>(otherwise_handler)());
      }
    }

    // Extracts bound reference directly from variant for direct bind
    // patterns.
    template <typename Pattern, typename Subject>
    constexpr decltype(auto)
    variant_direct_ref_bind_get(Subject &subject) {
      using pattern_t = std::decay_t<Pattern>;

      if constexpr (is_variant_direct_ref_bind_pattern_v<
                        pattern_t>) {
        if constexpr (is_variant_type_is_pattern_v<pattern_t>) {
          static_assert_variant_alt_unique<typename pattern_t::alt_t,
                                           Subject>();
          return (std::get<pattern_t::template alt_index<Subject>()>(
              subject));
        }
        else {
          constexpr std::size_t
              I = variant_type_alt_index<pattern_t>::value;
          static_assert_variant_alt_index<I, Subject>();
          return (std::get<I>(subject));
        }
      }
    }

    // Runtime matcher for simple variant patterns by active index.
    template <typename Pattern, typename Subject>
    constexpr bool
    simple_variant_pattern_matches_index(std::size_t active_index) {
      using pattern_t = std::decay_t<Pattern>;

      if constexpr (is_wildcard_pattern_v<pattern_t>) {
        return true;
      }
      else if constexpr (is_simple_variant_type_is_pattern_v<
                             pattern_t>) {
        static_assert_variant_alt_unique<typename pattern_t::alt_t,
                                         Subject>();
        return active_index
               == pattern_t::template alt_index<Subject>();
      }
      else if constexpr (is_simple_variant_type_alt_pattern_v<
                             pattern_t>) {
        constexpr std::size_t
            I = simple_variant_alt_index<pattern_t>::value;
        static_assert_variant_alt_index<I, Subject>();
        return active_index == I;
      }
      else {
        return false;
      }
    }

    template <typename GuardedPattern, typename BoundTuple>
    constexpr bool guard_predicate_holds(GuardedPattern &guarded,
                                         BoundTuple     &bound) {
      using bound_t                     = std::decay_t<BoundTuple>;
      constexpr std::size_t bound_arity = std::tuple_size_v<bound_t>;

      if constexpr (ptn::pat::traits::is_tuple_guard_predicate_v<
                        decltype(guarded.pred)>) {
        ptn::core::common::static_assert_tuple_guard_index<
            ptn::pat::mod::max_tuple_guard_index<
                std::decay_t<decltype(guarded.pred)>>::value,
            bound_arity>();
        return static_cast<bool>(guarded.pred(bound));
      }
      else if constexpr (ptn::pat::traits::is_guard_predicate_v<
                             decltype(guarded.pred)>) {
        ptn::core::common::static_assert_unary_guard_arity<
            bound_arity>();
        return static_cast<bool>(guarded.pred(std::get<0>(bound)));
      }
      else {
        return static_cast<bool>(std::apply(guarded.pred, bound));
      }
    }

    template <typename Pred, typename BoundRef>
    constexpr bool
    guard_predicate_holds_direct_ref(Pred     &pred,
                                     BoundRef &bound_ref) {
      auto bound_tuple = std::forward_as_tuple(bound_ref);

      if constexpr (ptn::pat::traits::is_tuple_guard_predicate_v<
                        Pred>) {
        ptn::core::common::static_assert_tuple_guard_index<
            ptn::pat::mod::max_tuple_guard_index<
                std::decay_t<Pred>>::value,
            1>();
        return static_cast<bool>(pred(bound_tuple));
      }
      else if constexpr (ptn::pat::traits::is_guard_predicate_v<
                             Pred>) {
        return static_cast<bool>(pred(bound_ref));
      }
      else {
        return static_cast<bool>(std::invoke(pred, bound_ref));
      }
    }

    template <typename Result,
              typename Case,
              typename Subject,
              typename OnMiss>
    constexpr Result eval_typed_case_or_else_variant_direct_ref_bind(
        Case &current_case, Subject &subject, OnMiss &&on_miss) {
      using case_t    = std::remove_reference_t<Case>;
      using pattern_t = traits::case_pattern_t<case_t>;

      if constexpr (detail::is_guarded_pattern_v<pattern_t>) {
        auto &guarded = current_case.pattern;
        using inner_t = typename guarded_inner_pattern<
            pattern_t>::type;

        if (guarded.inner.match(subject)) {
          auto &&bound_ref = variant_direct_ref_bind_get<inner_t>(
              subject);
          if (guard_predicate_holds_direct_ref(guarded.pred,
                                               bound_ref)) {
            return static_cast<Result>(
                std::invoke(current_case.handler, bound_ref));
          }
        }
      }
      else if (current_case.pattern.match(subject)) {
        auto &&bound_ref = variant_direct_ref_bind_get<pattern_t>(
            subject);
        return static_cast<Result>(
            std::invoke(current_case.handler, bound_ref));
      }

      return std::forward<OnMiss>(on_miss)();
    }

    template <typename Case,
              typename Subject,
              typename OnGuardHit,
              typename OnMiss>
    constexpr decltype(auto)
    eval_guarded_case(Case        &current_case,
                      Subject     &subject,
                      OnGuardHit &&on_guard_hit,
                      OnMiss     &&on_miss) {
      auto &guarded = current_case.pattern;
      if (guarded.inner.match(subject)) {
        auto &&bound = guarded.inner.bind(subject);
        if (guard_predicate_holds(guarded, bound)) {
          return std::forward<OnGuardHit>(on_guard_hit)(bound);
        }
      }

      return std::forward<OnMiss>(on_miss)();
    }

    template <typename Result, typename Case, typename Subject>
    constexpr Result invoke_typed_case_handler(Case    &current_case,
                                               Subject &subject) {
      using case_t       = std::remove_reference_t<Case>;
      using pattern_t    = traits::case_pattern_t<case_t>;
      using handler_t    = traits::case_handler_t<case_t>;
      using bound_args_t = pat::base::binding_args_t<pattern_t,
                                                     Subject>;

      constexpr bool
          zero_bind_noarg_handler = std::tuple_size_v<bound_args_t>
                                        == 0
                                    && std::is_invocable_v<
                                        handler_t>;

      if constexpr (zero_bind_noarg_handler) {
        return static_cast<Result>(current_case.handler());
      }
      else {
        return static_cast<Result>(
            invoke_handler(current_case, subject));
      }
    }

    template <typename Result,
              typename Case,
              typename Subject,
              typename OnMiss>
    constexpr Result eval_typed_case_or_else(Case    &current_case,
                                             Subject &subject,
                                             OnMiss &&on_miss) {
      using case_t    = std::remove_reference_t<Case>;
      using pattern_t = traits::case_pattern_t<case_t>;

      if constexpr (is_variant_subject_v<Subject>) {
        if constexpr (is_variant_direct_ref_bind_pattern_v<
                          pattern_t>) {
          return eval_typed_case_or_else_variant_direct_ref_bind<
              Result>(
              current_case, subject, std::forward<OnMiss>(on_miss));
        }
        else if constexpr (detail::is_guarded_pattern_v<pattern_t>) {
          using inner_t = typename guarded_inner_pattern<
              pattern_t>::type;
          if constexpr (is_variant_direct_ref_bind_pattern_v<
                            inner_t>) {
            return eval_typed_case_or_else_variant_direct_ref_bind<
                Result>(current_case,
                        subject,
                        std::forward<OnMiss>(on_miss));
          }
        }
      }

      if constexpr (detail::is_guarded_pattern_v<pattern_t>) {
        return eval_guarded_case(
            current_case,
            subject,
            [&](auto &bound) -> Result {
              return static_cast<Result>(detail::invoke_from_tuple(
                  current_case.handler, bound));
            },
            std::forward<OnMiss>(on_miss));
      }
      else if (case_matcher<case_t, Subject>::matches(current_case,
                                                      subject)) {
        return invoke_typed_case_handler<Result>(current_case,
                                                 subject);
      }
      else {
        return std::forward<OnMiss>(on_miss)();
      }
    }

    template <std::size_t CaseIndex,
              typename Result,
              typename CasesTuple>
    constexpr Result
    invoke_variant_simple_case_entry(CasesTuple &cases) {
      auto &current_case = std::get<CaseIndex>(cases);
      return static_cast<Result>(current_case.handler());
    }

    template <typename Metadata,
              typename Subject,
              typename CasesTuple,
              std::size_t I = 0,
              std::size_t N = std::tuple_size_v<
                  std::remove_reference_t<CasesTuple>>>
    constexpr void fill_static_literal_case_index_table(
        std::array<typename Metadata::case_index_t,
                   Metadata::range_size> &table) {
      if constexpr (I < N) {
        using tuple_t   = std::remove_reference_t<CasesTuple>;
        using case_t    = std::tuple_element_t<I, tuple_t>;
        using pattern_t = traits::case_pattern_t<case_t>;

        if constexpr (is_static_literal_pattern_v<pattern_t>) {
          constexpr typename Metadata::key_t value =
              static_literal_case_value<pattern_t, Subject>::value;
          constexpr std::size_t offset = static_cast<std::size_t>(
              value - Metadata::min_value);

          if (table[offset] == Metadata::k_invalid_case_index) {
            table[offset] = static_cast<typename Metadata::case_index_t>(I);
          }
        }

        fill_static_literal_case_index_table<Metadata,
                                             Subject,
                                             CasesTuple,
                                             I + 1,
                                             N>(table);
      }
    }

    template <typename Metadata, typename Subject, typename CasesTuple>
    constexpr auto make_static_literal_case_index_table() {
      std::array<typename Metadata::case_index_t, Metadata::range_size> table{};

      for (auto &entry : table) {
        entry = Metadata::k_invalid_case_index;
      }

      fill_static_literal_case_index_table<Metadata,
                                           Subject,
                                           CasesTuple>(table);
      return table;
    }

    template <typename Plan,
              typename Subject,
              typename CasesTuple,
              bool Enabled = Plan::metadata_t::use_dense_table>
    struct static_literal_dispatch_cache;

    template <typename Plan, typename Subject, typename CasesTuple>
    struct static_literal_dispatch_cache<Plan,
                                         Subject,
                                         CasesTuple,
                                         true> {
      using metadata_t = typename Plan::metadata_t;

      static constexpr auto case_index_table =
          make_static_literal_case_index_table<metadata_t,
                                               Subject,
                                               CasesTuple>();
    };

    template <std::size_t CaseIndex,
              typename Result,
              typename CasesTuple>
    constexpr Result
    invoke_static_literal_case_entry(CasesTuple &cases) {
      constexpr std::size_t case_count = std::tuple_size_v<
          std::remove_reference_t<CasesTuple>>;

      if constexpr (CaseIndex < case_count) {
        auto &current_case = std::get<CaseIndex>(cases);
        return static_cast<Result>(current_case.handler());
      }
      else {
        return unreachable_result<Result>();
      }
    }

    template <typename Result, typename CasesTuple>
    inline Result dispatch_static_literal_case_index(std::size_t case_index,
                                                     CasesTuple &cases) {
      switch (case_index) {
        PTN_DETAIL_STATIC_LITERAL_SWITCH_BLOCK_16(0);
        PTN_DETAIL_STATIC_LITERAL_SWITCH_BLOCK_16(16);
        PTN_DETAIL_STATIC_LITERAL_SWITCH_BLOCK_16(32);
        PTN_DETAIL_STATIC_LITERAL_SWITCH_BLOCK_16(48);
        PTN_DETAIL_STATIC_LITERAL_SWITCH_BLOCK_16(64);
        PTN_DETAIL_STATIC_LITERAL_SWITCH_BLOCK_16(80);
        PTN_DETAIL_STATIC_LITERAL_SWITCH_BLOCK_16(96);
        PTN_DETAIL_STATIC_LITERAL_SWITCH_BLOCK_16(112);
        PTN_DETAIL_STATIC_LITERAL_SWITCH_BLOCK_16(128);
        PTN_DETAIL_STATIC_LITERAL_SWITCH_BLOCK_16(144);
        PTN_DETAIL_STATIC_LITERAL_SWITCH_BLOCK_16(160);
        PTN_DETAIL_STATIC_LITERAL_SWITCH_BLOCK_16(176);
        PTN_DETAIL_STATIC_LITERAL_SWITCH_BLOCK_16(192);
        PTN_DETAIL_STATIC_LITERAL_SWITCH_BLOCK_16(208);
        PTN_DETAIL_STATIC_LITERAL_SWITCH_BLOCK_16(224);
        PTN_DETAIL_STATIC_LITERAL_SWITCH_BLOCK_16(240);
      default:
        return unreachable_result<Result>();
      }
    }

    template <typename Plan,
              typename Result,
              typename Subject,
              typename CasesTuple,
              typename Otherwise>
    inline Result eval_cases_impl_static_literal_dispatch(
        Subject    &subject,
        CasesTuple &cases,
        Otherwise &&otherwise_handler) {
      using metadata_t = typename Plan::metadata_t;
      using cache_t    = static_literal_dispatch_cache<Plan,
                                                    Subject,
                                                    CasesTuple>;
      using key_t      = typename Plan::key_t;

      const key_t value = static_cast<key_t>(subject);
      if (value >= Plan::min_value && value <= Plan::max_value) {
        const auto case_index = cache_t::case_index_table
            [static_cast<std::size_t>(value - Plan::min_value)];
        if (case_index != Plan::k_invalid_case_index) {
          return dispatch_static_literal_case_index<Result>(case_index,
                                                            cases);
        }
      }

      if constexpr (Plan::has_default_case) {
        return invoke_static_literal_case_entry<
            static_cast<std::size_t>(Plan::default_case_index),
            Result>(cases);
      }
      else {
        return invoke_otherwise_typed<Result>(
            subject, std::forward<Otherwise>(otherwise_handler));
      }
    }

    template <std::size_t I,
              typename Result,
              typename Subject,
              typename CasesTuple,
              typename Otherwise,
              std::size_t N = std::tuple_size_v<
                  std::remove_reference_t<CasesTuple>>>
    constexpr Result
    eval_cases_impl_typed(Subject    &subject,
                          CasesTuple &cases,
                          Otherwise &&otherwise_handler) {
      if constexpr (I >= N) {
        return invoke_otherwise_typed<Result>(
            subject, std::forward<Otherwise>(otherwise_handler));
      }
      else {
        auto &current_case = std::get<I>(cases);
        return eval_typed_case_or_else<Result>(
            current_case, subject, [&]() -> Result {
              return eval_cases_impl_typed<I + 1, Result>(
                  subject,
                  cases,
                  std::forward<Otherwise>(otherwise_handler));
            });
      }
    }

    template <std::size_t I,
              typename Result,
              typename Subject,
              typename CasesTuple,
              typename Otherwise,
              std::size_t N = std::tuple_size_v<
                  std::remove_reference_t<CasesTuple>>>
    constexpr Result eval_cases_impl_literal_simple_dispatch(
        Subject    &subject,
        CasesTuple &cases,
        Otherwise &&otherwise_handler) {
      if constexpr (I >= N) {
        return invoke_otherwise_typed<Result>(
            subject, std::forward<Otherwise>(otherwise_handler));
      }
      else {
        auto &current_case = std::get<I>(cases);
        using case_t       = std::remove_reference_t<
                  decltype(current_case)>;
        using pattern_t = traits::case_pattern_t<case_t>;

        if constexpr (is_wildcard_pattern_v<pattern_t>) {
          return static_cast<Result>(current_case.handler());
        }
        else {
          if (subject == current_case.pattern.v) {
            return static_cast<Result>(current_case.handler());
          }

          return eval_cases_impl_literal_simple_dispatch<I + 1,
                                                         Result>(
              subject,
              cases,
              std::forward<Otherwise>(otherwise_handler));
        }
      }
    }

    template <std::size_t ActiveIndex,
              std::size_t BeginCaseIndex,
              std::size_t EndCaseIndex,
              std::size_t I,
              typename Result,
              typename Subject,
              typename CasesTuple,
              typename Otherwise>
    constexpr Result
    eval_cases_impl_typed_variant_for_alt_case_range(
        Subject    &subject,
        CasesTuple &cases,
        Otherwise &&otherwise_handler) {
      if constexpr (I >= EndCaseIndex
                    || BeginCaseIndex >= EndCaseIndex) {
        return invoke_otherwise_typed<Result>(
            subject, std::forward<Otherwise>(otherwise_handler));
      }
      else {
        auto &current_case = std::get<I>(cases);
        using case_t       = std::remove_reference_t<
                  decltype(current_case)>;
        using pattern_t = traits::case_pattern_t<case_t>;

        if constexpr (variant_pattern_maybe_matches_alt_index<
                          pattern_t,
                          Subject,
                          ActiveIndex>()) {
          return eval_typed_case_or_else<
              Result>(current_case, subject, [&]() -> Result {
            return eval_cases_impl_typed_variant_for_alt_case_range<
                ActiveIndex,
                BeginCaseIndex,
                EndCaseIndex,
                I + 1,
                Result>(subject,
                        cases,
                        std::forward<Otherwise>(otherwise_handler));
          });
        }
        else {
          return eval_cases_impl_typed_variant_for_alt_case_range<
              ActiveIndex,
              BeginCaseIndex,
              EndCaseIndex,
              I + 1,
              Result>(subject,
                      cases,
                      std::forward<Otherwise>(otherwise_handler));
        }
      }
    }

    template <typename Plan,
              std::size_t AltIndex,
              typename Result,
              typename Subject,
              typename CasesTuple,
              typename Otherwise>
    constexpr Result eval_cases_impl_variant_dispatch_for_alt_hit(
        Subject    &subject,
        CasesTuple &cases,
        Otherwise &&otherwise_handler) {
      if constexpr (Plan::kind == dispatch_plan_kind::variant_simple) {
        using entry_t = typename Plan::template case_entry<AltIndex>;

        if constexpr (entry_t::has_any) {
          return invoke_variant_simple_case_entry<
              static_cast<std::size_t>(entry_t::case_index),
              Result>(cases);
        }
        else {
          return invoke_otherwise_typed<Result>(
              subject, std::forward<Otherwise>(otherwise_handler));
        }
      }
      else if constexpr (Plan::kind
                         == dispatch_plan_kind::variant_alt_bucketed) {
        using bucket_t = typename Plan::template case_bucket<AltIndex>;

        if constexpr (bucket_t::has_any) {
          // Replays only the residual case slice selected by the planner for
          // this active alternative. This preserves first-match semantics
          // without scanning unrelated cases.
          return eval_cases_impl_typed_variant_for_alt_case_range<
              AltIndex,
              bucket_t::begin,
              bucket_t::end,
              bucket_t::begin,
              Result>(subject,
                      cases,
                      std::forward<Otherwise>(otherwise_handler));
        }
        else {
          return invoke_otherwise_typed<Result>(
              subject, std::forward<Otherwise>(otherwise_handler));
        }
      }
      else {
        return unreachable_result<Result>();
      }
    }

    template <typename Plan,
              std::size_t ActiveIndex,
              typename Result,
              typename Subject,
              typename CasesTuple,
              typename Otherwise>
    constexpr Result eval_cases_impl_variant_dispatch_by_alt_inline(
        std::size_t active_index,
        Subject    &subject,
        CasesTuple &cases,
        Otherwise &&otherwise_handler) {
      if constexpr (ActiveIndex >= Plan::alt_count) {
        // Handles valueless_by_exception() and unsupported indexes.
        return invoke_otherwise_typed<Result>(
            subject, std::forward<Otherwise>(otherwise_handler));
      }
      else {
        if (active_index == ActiveIndex) {
          return eval_cases_impl_variant_dispatch_for_alt_hit<
              Plan,
              ActiveIndex,
              Result>(subject,
                      cases,
                      std::forward<Otherwise>(otherwise_handler));
        }

        return eval_cases_impl_variant_dispatch_by_alt_inline<
            Plan,
            ActiveIndex + 1,
            Result>(active_index,
                    subject,
                    cases,
                    std::forward<Otherwise>(otherwise_handler));
      }
    }

    template <typename Plan,
              std::size_t AltIndex,
              typename Result,
              typename Subject,
              typename CasesTuple,
              typename OtherwiseHandler>
    struct variant_dispatch_trampoline {
      static Result invoke(Subject          &subject,
                           CasesTuple       &cases,
                           OtherwiseHandler &otherwise_handler) {
        return eval_cases_impl_variant_dispatch_for_alt_hit<
            Plan,
            AltIndex,
            Result>(subject, cases, otherwise_handler);
      }
    };

    // Builds a dense cold-path trampoline table from the plan's compact map.
    // Only alternatives that survive planning receive a slot.
    template <typename Plan,
              typename Result,
              typename Subject,
              typename CasesTuple,
              typename OtherwiseHandler,
              std::size_t UsedAltCount,
              std::size_t... AltIndex>
    constexpr auto make_variant_dispatch_compact_table(
        std::index_sequence<AltIndex...>) {
      using dispatch_fn_t = Result (*)(Subject &,
                                       CasesTuple &,
                                       OtherwiseHandler &);
      std::array<dispatch_fn_t, UsedAltCount> table{};

      std::size_t next = 0;
      (void) std::initializer_list<int>{
          (Plan::compact_alt_index_map[AltIndex]
                   != Plan::k_invalid_compact_index
               ? (table[next++] = &variant_dispatch_trampoline<
                                  Plan,
                                  AltIndex,
                                  Result,
                                  Subject,
                                  CasesTuple,
                                  OtherwiseHandler>::invoke,
                  0)
               : 0)...};

      return table;
    }

    // Caches the cold-path tables derived from the selected variant plan.
    // Hot and warm tiers bypass this structure entirely.
    template <typename Plan,
              typename Result,
              typename Subject,
              typename CasesTuple,
              typename OtherwiseHandler>
    struct variant_dispatch_cache {
      using compact_index_t = typename Plan::compact_index_t;
      static constexpr compact_index_t
          k_invalid_compact_index = Plan::k_invalid_compact_index;
      static constexpr auto
          compact_alt_index_map = Plan::compact_alt_index_map;

      static constexpr auto compact_dispatch_table =
          make_variant_dispatch_compact_table<Plan,
                                              Result,
                                              Subject,
                                              CasesTuple,
                                              OtherwiseHandler,
                                              Plan::used_alt_count>(
              std::make_index_sequence<Plan::alt_count>{});
    };

    template <typename Plan,
              typename Result,
              typename Subject,
              typename CasesTuple,
              typename Otherwise>
    PTN_DETAIL_NOINLINE inline Result eval_cases_impl_variant_dispatch_cold(
        std::size_t active_index,
        Subject    &subject,
        CasesTuple &cases,
        Otherwise &&otherwise_handler) {
      using otherwise_handler_t = std::remove_reference_t<Otherwise>;
      using cache_t             = variant_dispatch_cache<
                      Plan,
                      Result,
                      Subject,
                      CasesTuple,
                      otherwise_handler_t>;

      // First maps the full variant index to the compact trampoline slot.
      const auto compact_index = cache_t::compact_alt_index_map[active_index];
      if (compact_index == cache_t::k_invalid_compact_index) {
        return invoke_otherwise_typed<Result>(
            subject, std::forward<Otherwise>(otherwise_handler));
      }

      otherwise_handler_t &otherwise_ref = otherwise_handler;
      return cache_t::compact_dispatch_table
          [static_cast<std::size_t>(compact_index)](
              subject, cases, otherwise_ref);
    }

    template <typename Plan,
              typename Result,
              typename Subject,
              typename CasesTuple,
              typename Otherwise>
    inline Result eval_cases_impl_variant_dispatch(
        std::size_t active_index,
        Subject    &subject,
        CasesTuple &cases,
        Otherwise &&otherwise_handler) {
      if (active_index >= Plan::alt_count) {
        return invoke_otherwise_typed<Result>(
            subject, std::forward<Otherwise>(otherwise_handler));
      }

      // Executes the same plan through different runtime shapes chosen by the
      // plan tier. The semantics are identical; only code size and branch
      // structure differ.
      if constexpr (Plan::tier == variant_dispatch_tier::hot_inline) {
        return eval_cases_impl_variant_dispatch_by_alt_inline<
            Plan,
            0,
            Result>(active_index,
                    subject,
                    cases,
                    std::forward<Otherwise>(otherwise_handler));
      }
      else if constexpr (Plan::tier
                         == variant_dispatch_tier::warm_segmented) {
        auto on_hit = [&](auto alt_index_c) -> Result {
          constexpr std::size_t
              alt_index = decltype(alt_index_c)::value;
          return eval_cases_impl_variant_dispatch_for_alt_hit<
              Plan,
              alt_index,
              Result>(subject,
                      cases,
                      std::forward<Otherwise>(otherwise_handler));
        };

        auto on_miss = [&]() -> Result {
          return invoke_otherwise_typed<Result>(
              subject, std::forward<Otherwise>(otherwise_handler));
        };

        return variant_binary_alt_dispatch<0,
                                           Plan::alt_count,
                                           Plan::alt_count>::dispatch(
            active_index, on_hit, on_miss);
      }
      else {
        return eval_cases_impl_variant_dispatch_cold<
            Plan,
            Result>(active_index,
                    subject,
                    cases,
                    std::forward<Otherwise>(otherwise_handler));
      }
    }

    // Executes a precomputed dispatch plan.
    // The planner owns dispatch selection; evaluator code here only maps the
    // selected shape to its runtime entry point.
    template <typename Plan,
              typename Result,
              typename Subject,
              typename CasesTuple,
              typename Otherwise>
    constexpr Result eval_cases_with_dispatch_plan(
        Subject    &subject,
        CasesTuple &cases,
        Otherwise &&otherwise_handler) {
      if constexpr (Plan::kind == dispatch_plan_kind::static_literal_dense) {
        return eval_cases_impl_static_literal_dispatch<Plan, Result>(
            subject,
            cases,
            std::forward<Otherwise>(otherwise_handler));
      }
      else if constexpr (Plan::kind == dispatch_plan_kind::literal_linear) {
        return eval_cases_impl_literal_simple_dispatch<0, Result>(
            subject,
            cases,
            std::forward<Otherwise>(otherwise_handler));
      }
      else if constexpr (Plan::kind == dispatch_plan_kind::variant_simple) {
        const std::size_t active_index = subject.index();
        return eval_cases_impl_variant_dispatch<
            Plan,
            Result>(active_index,
                    subject,
                    cases,
                    std::forward<Otherwise>(otherwise_handler));
      }
      else if constexpr (Plan::kind
                         == dispatch_plan_kind::variant_alt_bucketed) {
        const std::size_t active_index = subject.index();
        return eval_cases_impl_variant_dispatch<
            Plan,
            Result>(active_index,
                    subject,
                    cases,
                    std::forward<Otherwise>(otherwise_handler));
      }
      else {
        return eval_cases_impl_typed<0, Result>(
            subject,
            cases,
            std::forward<Otherwise>(otherwise_handler));
      }
    }
  } // namespace detail

  template <typename Result,
            typename Subject,
            typename CasesTuple,
            typename Otherwise>
  constexpr Result eval_cases_typed(Subject    &subject,
                                    CasesTuple &cases,
                                    Otherwise &&otherwise_handler) {
    using dispatch_plan_t = detail::dispatch_plan<Subject,
                                                  CasesTuple>;

    return detail::eval_cases_with_dispatch_plan<
        dispatch_plan_t,
        Result>(
        subject,
        cases,
        std::forward<Otherwise>(otherwise_handler));
  }
} // namespace ptn::core::common

#undef PTN_DETAIL_NOINLINE
#undef PTN_DETAIL_STATIC_LITERAL_SWITCH_BLOCK_16
#undef PTN_DETAIL_STATIC_LITERAL_SWITCH_CASE
