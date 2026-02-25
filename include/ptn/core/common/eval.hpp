#pragma once

// Common evaluation logic for matching and handler invocation.
//
// This header provides the core runtime evaluation machinery for pattern
// matching, including case matching logic, handler invocation with bound
// values, and sequential case evaluation.

#include <tuple>
#include <type_traits>
#include <utility>
#include <array>
#include <functional>
#include <cstdlib>
#include <variant>

#include "ptn/core/common/common_traits.hpp"
#include "ptn/core/common/diagnostics.hpp"
#include "ptn/core/common/optimize.hpp"

namespace ptn::core::common {

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
    struct has_match_member<P,
                            S,
                            typename void_type<decltype(std::declval<P>().match(
                                std::declval<S>()))>::type> : std::true_type {};

    template <typename P, typename S>
    constexpr bool has_match_member_v = has_match_member<P, S>::value;

    // Detects if a pattern has a .bind(subject) member function
    template <typename P, typename S, typename = void>
    struct has_bind_member : std::false_type {};

    template <typename P, typename S>
    struct has_bind_member<P,
                           S,
                           typename void_type<decltype(std::declval<P>().bind(
                               std::declval<S>()))>::type> : std::true_type {};

    template <typename P, typename S>
    constexpr bool has_bind_member_v = has_bind_member<P, S>::value;
  } // namespace detail

  // Case matching.

  // Checks if a case's pattern matches a subject value
  // Supports both class-based patterns with .match() and functional patterns
  template <typename Case, typename Subject>
  struct case_matcher {
    using case_type    = Case;
    using subject_type = Subject;
    using pattern_type = traits::case_pattern_t<Case>;

    // Returns true if the pattern matches the subject
    static constexpr bool matches(const case_type    &c,
                                  const subject_type &subject) {
      if constexpr (detail::has_match_member_v<pattern_type, subject_type>) {
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
    constexpr decltype(auto)
    invoke_from_tuple_impl(F &&f, Tuple &&t, std::index_sequence<I...>) {

      using result_t = std::
          invoke_result_t<F, decltype(std::get<I>(std::forward<Tuple>(t)))...>;

      if constexpr (std::is_void_v<result_t>) {
        std::invoke(std::forward<F>(f), std::get<I>(std::forward<Tuple>(t))...);
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
  // Design A: handler receives only the bound values from pattern matching
  //   handler( pattern.bind(subject)... )
  //   The subject itself is not passed as an extra first argument
  template <typename Case, typename Subject>
  constexpr decltype(auto) invoke_handler(const Case &c, Subject &subject) {
    using pattern_type = traits::case_pattern_t<Case>;

    ptn::core::common::static_assert_pattern_has_bind<
        detail::has_bind_member_v<pattern_type, Subject &>>();

    // Extract bound values from the pattern
    auto &&bound_values = c.pattern.bind(subject);
    return detail::invoke_from_tuple(c.handler, bound_values);
  }

  // Case-sequence evaluation.
  namespace detail {
    // Recursive implementation for evaluating a tuple of cases in sequence
    template <
        std::size_t I,
        typename Subject,
        typename CasesTuple,
        typename Otherwise,
        std::size_t N = std::tuple_size_v<std::remove_reference_t<CasesTuple>>>
    constexpr decltype(auto) eval_cases_impl(Subject    &subject,
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
        using case_t       = std::remove_reference_t<decltype(current_case)>;

        if (case_matcher<case_t, Subject>::matches(current_case, subject)) {
          // Found a match, invoke its handler
          return invoke_handler(current_case, subject);
        }
        else {
          // Try the next case.
          return eval_cases_impl<I + 1>(
              subject, cases, std::forward<Otherwise>(otherwise_handler));
        }
      }
    }
  } // namespace detail

  // Public entry point to evaluate a sequence of cases against a subject
  // Tries each case in order, invoking the first matching handler
  template <typename Subject, typename CasesTuple, typename Otherwise>
  constexpr decltype(auto) eval_cases(Subject    &subject,
                                      CasesTuple &cases,
                                      Otherwise &&otherwise_handler) {
    using tuple_t           = std::remove_reference_t<CasesTuple>;
    constexpr std::size_t N = std::tuple_size_v<tuple_t>;

    (void) N; // silence unused warning if needed

    return detail::eval_cases_impl<0>(
        subject, cases, std::forward<Otherwise>(otherwise_handler));
  }

  // Strongly-typed evaluation entry point.
  // This overload forces the match evaluation to return a single, precomputed
  // result type. This is required for expression-style matches where different
  // branches may produce different (but compatible) return types.
  namespace detail {
    template <typename Result>
    [[noreturn]] inline Result unreachable_result() {
      std::abort();
    }

    template <typename Result, typename Subject, typename Otherwise>
    constexpr Result invoke_otherwise_typed(Subject    &subject,
                                            Otherwise &&otherwise_handler) {
      // Shared typed otherwise flow for both generic and fast-path evaluators.
      using otherwise_result = traits::otherwise_result_t<Otherwise, Subject>;

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

    template <typename GuardedPattern, typename BoundTuple>
    constexpr bool
    guard_predicate_holds(GuardedPattern &guarded, BoundTuple &bound) {
      using bound_t = std::decay_t<BoundTuple>;
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
        ptn::core::common::static_assert_unary_guard_arity<bound_arity>();
        return static_cast<bool>(guarded.pred(std::get<0>(bound)));
      }
      else {
        return static_cast<bool>(std::apply(guarded.pred, bound));
      }
    }

    template <typename Case,
              typename Subject,
              typename OnGuardHit,
              typename OnMiss>
    constexpr decltype(auto)
    eval_guarded_case(Case       &current_case,
                      Subject    &subject,
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
    constexpr Result invoke_typed_case_handler(Case &current_case,
                                               Subject &subject) {
      using case_t       = std::remove_reference_t<Case>;
      using pattern_t    = traits::case_pattern_t<case_t>;
      using handler_t    = traits::case_handler_t<case_t>;
      using bound_args_t = pat::base::binding_args_t<pattern_t, Subject>;

      constexpr bool zero_bind_noarg_handler =
          std::tuple_size_v<bound_args_t> == 0
          && std::is_invocable_v<handler_t>;

      if constexpr (zero_bind_noarg_handler) {
        return static_cast<Result>(current_case.handler());
      }
      else {
        return static_cast<Result>(invoke_handler(current_case, subject));
      }
    }

    template <typename Result, typename Case, typename Subject, typename OnMiss>
    constexpr Result eval_typed_case_or_else(Case    &current_case,
                                             Subject &subject,
                                             OnMiss  &&on_miss) {
      using case_t    = std::remove_reference_t<Case>;
      using pattern_t = traits::case_pattern_t<case_t>;

      if constexpr (detail::is_guarded_pattern_v<pattern_t>) {
        return eval_guarded_case(
            current_case,
            subject,
            [&](auto &bound) -> Result {
              return static_cast<Result>(
                  detail::invoke_from_tuple(current_case.handler, bound));
            },
            std::forward<OnMiss>(on_miss));
      }
      else if (case_matcher<case_t, Subject>::matches(current_case, subject)) {
        return invoke_typed_case_handler<Result>(current_case, subject);
      }
      else {
        return std::forward<OnMiss>(on_miss)();
      }
    }

    template <
        std::size_t I,
        typename Result,
        typename Subject,
        typename CasesTuple,
        typename Otherwise,
        std::size_t N = std::tuple_size_v<std::remove_reference_t<CasesTuple>>>
    constexpr Result
    eval_cases_impl_variant_simple_dispatch(std::size_t active_index,
                                            Subject    &subject,
                                            CasesTuple &cases,
                                            Otherwise &&otherwise_handler) {
      // Single-dispatch evaluator for simple variant-only case chains.
      if constexpr (I >= N) {
        return invoke_otherwise_typed<Result>(
            subject, std::forward<Otherwise>(otherwise_handler));
      }
      else {
        auto &current_case = std::get<I>(cases);
        using case_t       = std::remove_reference_t<decltype(current_case)>;
        using pattern_t    = traits::case_pattern_t<case_t>;

        if (simple_variant_pattern_matches_index<pattern_t, Subject>(
                active_index)) {
          return static_cast<Result>(current_case.handler());
        }

        return eval_cases_impl_variant_simple_dispatch<I + 1, Result>(
            active_index,
            subject,
            cases,
            std::forward<Otherwise>(otherwise_handler));
      }
    }

    template <
        std::size_t ActiveIndex,
        std::size_t I,
        typename Result,
        typename Subject,
        typename CasesTuple,
        typename Otherwise,
        std::size_t N = std::tuple_size_v<std::remove_reference_t<CasesTuple>>>
    constexpr Result
    eval_cases_impl_variant_simple_for_alt(Subject    &subject,
                                           CasesTuple &cases,
                                           Otherwise &&otherwise_handler) {
      if constexpr (I >= N) {
        return invoke_otherwise_typed<Result>(
            subject, std::forward<Otherwise>(otherwise_handler));
      }
      else {
        auto &current_case = std::get<I>(cases);
        using case_t       = std::remove_reference_t<decltype(current_case)>;
        using pattern_t    = traits::case_pattern_t<case_t>;

        if constexpr (simple_variant_pattern_matches_alt_index<pattern_t,
                                                               Subject,
                                                               ActiveIndex>()) {
          return static_cast<Result>(current_case.handler());
        }
        else {
          return eval_cases_impl_variant_simple_for_alt<ActiveIndex,
                                                        I + 1,
                                                        Result>(
              subject, cases, std::forward<Otherwise>(otherwise_handler));
        }
      }
    }

    template <std::size_t ActiveIndex,
              typename Result,
              typename Subject,
              typename CasesTuple,
              typename OtherwiseHandler>
    constexpr Result
    eval_cases_impl_variant_simple_for_alt_entry(Subject          &subject,
                                                 CasesTuple       &cases,
                                                 OtherwiseHandler &otherwise_handler) {
      return eval_cases_impl_variant_simple_for_alt<ActiveIndex, 0, Result>(
          subject, cases, otherwise_handler);
    }

    template <typename Result,
              typename Subject,
              typename CasesTuple,
              typename OtherwiseHandler,
              std::size_t... AltIndex>
    constexpr auto
    make_variant_simple_dispatch_table(std::index_sequence<AltIndex...>) {
      using dispatch_fn_t =
          Result (*)(Subject &, CasesTuple &, OtherwiseHandler &);

      return std::array<dispatch_fn_t, sizeof...(AltIndex)>{
          &eval_cases_impl_variant_simple_for_alt_entry<AltIndex,
                                                        Result,
                                                        Subject,
                                                        CasesTuple,
                                                        OtherwiseHandler>...};
    }

    template <
        typename Result,
        typename Subject,
        typename CasesTuple,
        typename Otherwise,
        typename SubjectValue =
            std::remove_cv_t<std::remove_reference_t<Subject>>,
        std::size_t AltCount = std::variant_size_v<SubjectValue>>
    inline Result eval_cases_impl_variant_simple_dispatch_by_alt_table(
        std::size_t active_index,
        Subject    &subject,
        CasesTuple &cases,
        Otherwise &&otherwise_handler) {
      if (active_index >= AltCount) {
        return invoke_otherwise_typed<Result>(
            subject, std::forward<Otherwise>(otherwise_handler));
      }

      using otherwise_handler_t = std::remove_reference_t<Otherwise>;
      static constexpr auto dispatch_table =
          make_variant_simple_dispatch_table<Result,
                                             Subject,
                                             CasesTuple,
                                             otherwise_handler_t>(
              std::make_index_sequence<AltCount>{});

      otherwise_handler_t &otherwise_ref = otherwise_handler;
      return dispatch_table[active_index](subject, cases, otherwise_ref);
    }

    template <
        std::size_t I,
        typename Result,
        typename Subject,
        typename CasesTuple,
        typename Otherwise,
        std::size_t N = std::tuple_size_v<std::remove_reference_t<CasesTuple>>>
    constexpr Result eval_cases_impl_typed(Subject    &subject,
                                           CasesTuple &cases,
                                           Otherwise &&otherwise_handler) {
      if constexpr (I >= N) {
        return invoke_otherwise_typed<Result>(
            subject, std::forward<Otherwise>(otherwise_handler));
      }
      else {
        auto &current_case = std::get<I>(cases);
        return eval_typed_case_or_else<Result>(
            current_case,
            subject,
            [&]() -> Result {
              return eval_cases_impl_typed<I + 1, Result>(
                  subject, cases, std::forward<Otherwise>(otherwise_handler));
            });
      }
    }

    template <
        std::size_t I,
        typename Result,
        typename Subject,
        typename CasesTuple,
        typename Otherwise,
        std::size_t N = std::tuple_size_v<std::remove_reference_t<CasesTuple>>>
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
        using case_t       = std::remove_reference_t<decltype(current_case)>;
        using pattern_t    = traits::case_pattern_t<case_t>;

        if constexpr (is_wildcard_pattern_v<pattern_t>) {
          return static_cast<Result>(current_case.handler());
        }
        else {
          if (subject == current_case.pattern.v) {
            return static_cast<Result>(current_case.handler());
          }

          return eval_cases_impl_literal_simple_dispatch<I + 1, Result>(
              subject, cases, std::forward<Otherwise>(otherwise_handler));
        }
      }
    }

    template <
        std::size_t I,
        typename Result,
        typename Subject,
        typename CasesTuple,
        typename Otherwise,
        std::size_t N = std::tuple_size_v<std::remove_reference_t<CasesTuple>>>
    constexpr Result
    eval_cases_impl_typed_variant_prefilter(std::size_t active_index,
                                            Subject    &subject,
                                            CasesTuple &cases,
                                            Otherwise &&otherwise_handler) {
      if constexpr (I >= N) {
        return invoke_otherwise_typed<Result>(
            subject, std::forward<Otherwise>(otherwise_handler));
      }
      else {
        auto &current_case = std::get<I>(cases);
        using case_t       = std::remove_reference_t<decltype(current_case)>;
        using pattern_t    = traits::case_pattern_t<case_t>;

        if (!variant_pattern_maybe_matches_active_index<pattern_t, Subject>(
                active_index)) {
          return eval_cases_impl_typed_variant_prefilter<I + 1, Result>(
              active_index,
              subject,
              cases,
              std::forward<Otherwise>(otherwise_handler));
        }

        return eval_typed_case_or_else<Result>(
            current_case,
            subject,
            [&]() -> Result {
              return eval_cases_impl_typed_variant_prefilter<I + 1, Result>(
                  active_index,
                  subject,
                  cases,
                  std::forward<Otherwise>(otherwise_handler));
            });
      }
    }

    template <
        std::size_t ActiveIndex,
        std::size_t I,
        typename Result,
        typename Subject,
        typename CasesTuple,
        typename Otherwise,
        std::size_t N = std::tuple_size_v<std::remove_reference_t<CasesTuple>>>
    constexpr Result
    eval_cases_impl_typed_variant_for_alt(Subject    &subject,
                                          CasesTuple &cases,
                                          Otherwise &&otherwise_handler) {
      if constexpr (I >= N) {
        return invoke_otherwise_typed<Result>(
            subject, std::forward<Otherwise>(otherwise_handler));
      }
      else {
        auto &current_case = std::get<I>(cases);
        using case_t       = std::remove_reference_t<decltype(current_case)>;
        using pattern_t    = traits::case_pattern_t<case_t>;

        if constexpr (variant_pattern_maybe_matches_alt_index<pattern_t,
                                                              Subject,
                                                              ActiveIndex>()) {
          return eval_typed_case_or_else<Result>(
              current_case,
              subject,
              [&]() -> Result {
                return eval_cases_impl_typed_variant_for_alt<ActiveIndex,
                                                             I + 1,
                                                             Result>(
                    subject, cases, std::forward<Otherwise>(otherwise_handler));
              });
        }
        else {
          return eval_cases_impl_typed_variant_for_alt<ActiveIndex, I + 1, Result>(
              subject, cases, std::forward<Otherwise>(otherwise_handler));
        }
      }
    }

    template <
        std::size_t ActiveIndex,
        typename Result,
        typename Subject,
        typename CasesTuple,
        typename Otherwise,
        typename SubjectValue =
            std::remove_cv_t<std::remove_reference_t<Subject>>,
        std::size_t AltCount = std::variant_size_v<SubjectValue>>
    constexpr Result
    eval_cases_impl_typed_variant_dispatch_by_alt(std::size_t active_index,
                                                  Subject    &subject,
                                                  CasesTuple &cases,
                                                  Otherwise &&otherwise_handler) {
      if constexpr (ActiveIndex >= AltCount) {
        // Handles valueless_by_exception() and unsupported indexes.
        return invoke_otherwise_typed<Result>(
            subject, std::forward<Otherwise>(otherwise_handler));
      }
      else {
        if (active_index == ActiveIndex) {
          return eval_cases_impl_typed_variant_for_alt<ActiveIndex, 0, Result>(
              subject, cases, std::forward<Otherwise>(otherwise_handler));
        }

        return eval_cases_impl_typed_variant_dispatch_by_alt<ActiveIndex + 1,
                                                             Result>(
            active_index,
            subject,
            cases,
            std::forward<Otherwise>(otherwise_handler));
      }
    }

    template <std::size_t ActiveIndex,
              typename Result,
              typename Subject,
              typename CasesTuple,
              typename OtherwiseHandler>
    constexpr Result
    eval_cases_impl_typed_variant_for_alt_entry(Subject          &subject,
                                                CasesTuple       &cases,
                                                OtherwiseHandler &otherwise_handler) {
      return eval_cases_impl_typed_variant_for_alt<ActiveIndex, 0, Result>(
          subject, cases, otherwise_handler);
    }

    template <typename Result,
              typename Subject,
              typename CasesTuple,
              typename OtherwiseHandler,
              std::size_t... AltIndex>
    constexpr auto
    make_typed_variant_dispatch_table(std::index_sequence<AltIndex...>) {
      using dispatch_fn_t =
          Result (*)(Subject &, CasesTuple &, OtherwiseHandler &);

      return std::array<dispatch_fn_t, sizeof...(AltIndex)>{
          &eval_cases_impl_typed_variant_for_alt_entry<AltIndex,
                                                       Result,
                                                       Subject,
                                                       CasesTuple,
                                                       OtherwiseHandler>...};
    }

    template <
        typename Result,
        typename Subject,
        typename CasesTuple,
        typename Otherwise,
        typename SubjectValue =
            std::remove_cv_t<std::remove_reference_t<Subject>>,
        std::size_t AltCount = std::variant_size_v<SubjectValue>>
    inline Result eval_cases_impl_typed_variant_dispatch_by_alt_table(
        std::size_t active_index,
        Subject    &subject,
        CasesTuple &cases,
        Otherwise &&otherwise_handler) {
      if (active_index >= AltCount) {
        return invoke_otherwise_typed<Result>(
            subject, std::forward<Otherwise>(otherwise_handler));
      }

      using otherwise_handler_t = std::remove_reference_t<Otherwise>;
      static constexpr auto dispatch_table =
          make_typed_variant_dispatch_table<Result,
                                            Subject,
                                            CasesTuple,
                                            otherwise_handler_t>(
              std::make_index_sequence<AltCount>{});

      otherwise_handler_t &otherwise_ref = otherwise_handler;
      return dispatch_table[active_index](subject, cases, otherwise_ref);
    }
  } // namespace detail

  template <typename Result,
            typename Subject,
            typename CasesTuple,
            typename Otherwise>
  constexpr Result eval_cases_typed(Subject    &subject,
                                    CasesTuple &cases,
                                    Otherwise &&otherwise_handler) {
    using dispatch_policy_t = detail::dispatch_policy<Subject, CasesTuple>;

    if constexpr (dispatch_policy_t::use_simple_literal_dispatch) {
      // Literal simple-dispatch fast path:
      // direct value checks for lit(...) + wildcard chains.
      return detail::eval_cases_impl_literal_simple_dispatch<0, Result>(
          subject, cases, std::forward<Otherwise>(otherwise_handler));
    }
    else if constexpr (dispatch_policy_t::use_simple_variant_dispatch) {
      // Variant simple-dispatch fast path:
      // read index once, then jump directly to active alt evaluator.
      const std::size_t active_index = subject.index();
      return detail::eval_cases_impl_variant_simple_dispatch_by_alt_table<Result>(
          active_index,
          subject,
          cases,
          std::forward<Otherwise>(otherwise_handler));
    }
    else if constexpr (dispatch_policy_t::use_variant_alt_dispatch) {
      // Mixed variant keyed path:
      // jump to active alternative, then evaluate only relevant cases.
      const std::size_t active_index = subject.index();
      return detail::eval_cases_impl_typed_variant_dispatch_by_alt_table<Result>(
          active_index,
          subject,
          cases,
          std::forward<Otherwise>(otherwise_handler));
    }
    else {
      return detail::eval_cases_impl_typed<0, Result>(
          subject, cases, std::forward<Otherwise>(otherwise_handler));
    }
  }
} // namespace ptn::core::common
