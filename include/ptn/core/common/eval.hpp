#pragma once

// Common evaluation logic for matching and handler invocation.
//
// This header provides the core runtime evaluation machinery for pattern
// matching, including case matching logic, handler invocation with bound
// values, and sequential case evaluation.

#include <tuple>
#include <type_traits>
#include <utility>
#include <functional>
#include <cstdlib>
#include <variant>

#include "ptn/core/common/common_traits.hpp"
#include "ptn/core/common/diagnostics.hpp"

namespace ptn::pat::type::detail {
  // Forward declarations used by variant fast-path traits.
  struct no_subpattern;

  template <typename T, typename SubPattern>
  struct type_is_pattern;

  template <std::size_t I, typename SubPattern>
  struct type_alt_pattern;
} // namespace ptn::pat::type::detail

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

    template <typename T>
    struct is_guarded_pattern : std::false_type {};

    template <typename Inner, typename Pred>
    struct is_guarded_pattern<ptn::pat::mod::guarded_pattern<Inner, Pred>>
        : std::true_type {};

    template <typename T>
    constexpr bool
        is_guarded_pattern_v = is_guarded_pattern<std::decay_t<T>>::value;

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

    // Wildcard can act as default arm in the simple variant dispatcher.
    template <typename Pattern>
    constexpr bool
        is_wildcard_pattern_v = std::is_same_v<std::decay_t<Pattern>,
                                               ptn::pat::detail::wildcard_t>;

    template <typename Pattern>
    constexpr bool is_simple_variant_dispatch_pattern_v =
        is_simple_variant_type_is_pattern_v<Pattern>
        || is_simple_variant_type_alt_pattern_v<Pattern>
        || is_wildcard_pattern_v<Pattern>;

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

    // Runtime index prefilter for mixed variant case chains.
    // Returns false only when a pattern is statically known to never match
    // the active variant alternative.
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
        using case_t       = std::remove_reference_t<decltype(current_case)>;
        using pattern_t    = traits::case_pattern_t<case_t>;
        using handler_t    = traits::case_handler_t<case_t>;
        using bound_args_t = pat::base::binding_args_t<pattern_t, Subject>;

        // Typed-eval fast path:
        // for zero-bind cases with no-arg handlers, avoid bind()+tuple
        // dispatch.
        constexpr bool
            zero_bind_noarg_handler = std::tuple_size_v<bound_args_t> == 0
                                      && std::is_invocable_v<handler_t>;

        if constexpr (detail::is_guarded_pattern_v<pattern_t>) {
          auto &guarded = current_case.pattern;
          if (guarded.inner.match(subject)) {
            auto &&bound                      = guarded.inner.bind(subject);
            using bound_t                     = std::decay_t<decltype(bound)>;
            constexpr std::size_t bound_arity = std::tuple_size_v<bound_t>;

            bool guard_ok = false;
            if constexpr (ptn::pat::traits::is_tuple_guard_predicate_v<
                              decltype(guarded.pred)>) {
              ptn::core::common::static_assert_tuple_guard_index<
                  ptn::pat::mod::max_tuple_guard_index<
                      std::decay_t<decltype(guarded.pred)>>::value,
                  bound_arity>();
              guard_ok = static_cast<bool>(guarded.pred(bound));
            }
            else if constexpr (ptn::pat::traits::is_guard_predicate_v<
                                   decltype(guarded.pred)>) {
              ptn::core::common::static_assert_unary_guard_arity<bound_arity>();
              guard_ok = static_cast<bool>(guarded.pred(std::get<0>(bound)));
            }
            else {
              guard_ok = static_cast<bool>(std::apply(guarded.pred, bound));
            }

            if (guard_ok) {
              return static_cast<Result>(
                  detail::invoke_from_tuple(current_case.handler, bound));
            }
          }

          return eval_cases_impl_typed<I + 1, Result>(
              subject, cases, std::forward<Otherwise>(otherwise_handler));
        }
        else if (case_matcher<case_t, Subject>::matches(current_case,
                                                        subject)) {
          if constexpr (zero_bind_noarg_handler) {
            // No binding payload to materialize; invoke directly.
            return static_cast<Result>(current_case.handler());
          }
          else {
            return static_cast<Result>(invoke_handler(current_case, subject));
          }
        }
        else {
          return eval_cases_impl_typed<I + 1, Result>(
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
        using handler_t    = traits::case_handler_t<case_t>;
        using bound_args_t = pat::base::binding_args_t<pattern_t, Subject>;

        if (!variant_pattern_maybe_matches_active_index<pattern_t, Subject>(
                active_index)) {
          return eval_cases_impl_typed_variant_prefilter<I + 1, Result>(
              active_index,
              subject,
              cases,
              std::forward<Otherwise>(otherwise_handler));
        }

        constexpr bool
            zero_bind_noarg_handler = std::tuple_size_v<bound_args_t> == 0
                                      && std::is_invocable_v<handler_t>;

        if constexpr (detail::is_guarded_pattern_v<pattern_t>) {
          auto &guarded = current_case.pattern;
          if (guarded.inner.match(subject)) {
            auto &&bound                      = guarded.inner.bind(subject);
            using bound_t                     = std::decay_t<decltype(bound)>;
            constexpr std::size_t bound_arity = std::tuple_size_v<bound_t>;

            bool guard_ok = false;
            if constexpr (ptn::pat::traits::is_tuple_guard_predicate_v<
                              decltype(guarded.pred)>) {
              ptn::core::common::static_assert_tuple_guard_index<
                  ptn::pat::mod::max_tuple_guard_index<
                      std::decay_t<decltype(guarded.pred)>>::value,
                  bound_arity>();
              guard_ok = static_cast<bool>(guarded.pred(bound));
            }
            else if constexpr (ptn::pat::traits::is_guard_predicate_v<
                                   decltype(guarded.pred)>) {
              ptn::core::common::static_assert_unary_guard_arity<bound_arity>();
              guard_ok = static_cast<bool>(guarded.pred(std::get<0>(bound)));
            }
            else {
              guard_ok = static_cast<bool>(std::apply(guarded.pred, bound));
            }

            if (guard_ok) {
              return static_cast<Result>(
                  detail::invoke_from_tuple(current_case.handler, bound));
            }
          }

          return eval_cases_impl_typed_variant_prefilter<I + 1, Result>(
              active_index,
              subject,
              cases,
              std::forward<Otherwise>(otherwise_handler));
        }
        else if (case_matcher<case_t, Subject>::matches(current_case,
                                                        subject)) {
          if constexpr (zero_bind_noarg_handler) {
            return static_cast<Result>(current_case.handler());
          }
          else {
            return static_cast<Result>(invoke_handler(current_case, subject));
          }
        }
        else {
          return eval_cases_impl_typed_variant_prefilter<I + 1, Result>(
              active_index,
              subject,
              cases,
              std::forward<Otherwise>(otherwise_handler));
        }
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
    using subject_t = std::remove_cv_t<std::remove_reference_t<Subject>>;

    constexpr bool use_simple_variant_dispatch =
        detail::is_variant_subject_v<subject_t>
        && detail::is_simple_variant_dispatch_cases_tuple_v<subject_t,
                                                            CasesTuple>;

    if constexpr (use_simple_variant_dispatch) {
      // Variant simple-dispatch fast path:
      // read index once, then resolve by case order (first-match-wins).
      const std::size_t active_index = subject.index();
      return detail::eval_cases_impl_variant_simple_dispatch<0, Result>(
          active_index,
          subject,
          cases,
          std::forward<Otherwise>(otherwise_handler));
    }
    else if constexpr (detail::is_variant_subject_v<subject_t>) {
      // Mixed variant path (e.g. simple + guarded): prefilter by active
      // alternative before evaluating each case.
      const std::size_t active_index = subject.index();
      return detail::eval_cases_impl_typed_variant_prefilter<0, Result>(
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
