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
#include "ptn/core/common/index_switch.hpp"
#include "ptn/core/common/inline.hpp"
#include "ptn/core/common/optimize.hpp"
#include "ptn/core/dsl/detail/case_expr_impl.hpp"

namespace ptn::core::common {
#if defined(_MSC_VER)
#define PTN_DETAIL_NOINLINE __declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
#define PTN_DETAIL_NOINLINE __attribute__((noinline))
#else
#define PTN_DETAIL_NOINLINE
#endif

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

    // Forward each element independently. Casting the tuple to an
    // rvalue exposes rvalue elements without moving the tuple
    // itself.
    template <std::size_t I, typename Tuple>
    PTN_DETAIL_ALWAYS_INLINE constexpr decltype(auto)
    forward_bound_element(std::remove_reference_t<Tuple> &tuple) {
      return std::get<I>(std::forward<Tuple>(tuple));
    }

    // MSVC-friendly tuple invocation
    // Avoids: std::apply + dependent lambda + if constexpr
    template <typename F, typename Tuple, std::size_t... I>
    constexpr decltype(auto) invoke_from_tuple_impl(
        F &&f, Tuple &&t, std::index_sequence<I...>) {

      using result_t = std::invoke_result_t<
          F,
          decltype(forward_bound_element<I, Tuple>(t))...>;

      if constexpr (std::is_void_v<result_t>) {
        std::invoke(std::forward<F>(f),
                    forward_bound_element<I, Tuple>(t)...);
        return;
      }
      else {
        return std::invoke(std::forward<F>(f),
                           forward_bound_element<I, Tuple>(t)...);
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
  constexpr decltype(auto) invoke_handler(Case    &c,
                                          Subject &subject) {
    using pattern_type = traits::case_pattern_t<Case>;

    ptn::core::common::static_assert_pattern_has_bind<
        detail::has_bind_member_v<pattern_type, Subject &>>();

    // Extract bound values from the pattern
    auto &&bound_values = std::as_const(c.pattern).bind(subject);
    return detail::invoke_from_tuple(
        c.handler,
        std::forward<decltype(bound_values)>(bound_values));
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
        auto &current_case = ptn::core::dsl::detail::case_get<I>(
            cases);
        using case_t = std::remove_reference_t<
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
      // Shared typed otherwise flow for both generic and lowered
      // evaluators.
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
      else {
        return static_cast<bool>(
            invoke_from_tuple(guarded.pred, bound));
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
          return std::forward<OnGuardHit>(on_guard_hit)(
              std::forward<decltype(bound)>(bound));
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

    template <std::size_t CaseIndex,
              typename Result,
              typename CasesTuple>
    PTN_DETAIL_ALWAYS_INLINE constexpr Result
    invoke_direct_case_entry(CasesTuple &&cases) {
      using tuple_t = std::remove_reference_t<CasesTuple>;
      constexpr std::size_t case_count = std::tuple_size_v<tuple_t>;

      if constexpr (CaseIndex < case_count) {
        using case_t    = std::tuple_element_t<CaseIndex, tuple_t>;
        using handler_t = traits::case_handler_t<case_t>;

        // Reuses the case object directly so temporary `on(...)`
        // packs can stay on the rvalue fast path when the selected
        // plan does not need general matching state.
        auto &&current_case = ptn::core::dsl::detail::case_get<
            CaseIndex>(std::forward<CasesTuple>(cases));

        if constexpr (core::dsl::detail::is_value_handler_v<
                          handler_t>) {
          return static_cast<Result>(current_case.handler.value);
        }
        else {
          return static_cast<Result>(current_case.handler());
        }
      }
      else {
        return unreachable_result<Result>();
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
            [&](auto &&bound) -> Result {
              return static_cast<Result>(detail::invoke_from_tuple(
                  current_case.handler,
                  std::forward<decltype(bound)>(bound)));
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

    // Alternative tested by a pattern whose only test on a variant
    // is "the active alternative is this one": `is<T>()`,
    // `alt<I>()`, their direct reference bindings, and `$(...)`
    // around the first two. `binds` says whether the pattern binds
    // exactly the active alternative, so the binding can be taken
    // without the pattern.
    // `index` is meaningful only when `value` is true. It is defined
    // here too so that `$(p)` around any `p` can read it: MSVC
    // evaluates that initializer whenever the class is instantiated.
    template <typename Pattern, typename Subject, typename = void>
    struct variant_alt_only_pattern {
      static constexpr bool        value = false;
      static constexpr bool        binds = false;
      static constexpr std::size_t index = 0;
    };

    template <typename Pattern, typename Subject>
    struct variant_alt_only_pattern<
        Pattern,
        Subject,
        std::enable_if_t<is_simple_variant_type_is_pattern_v<Pattern>
                         || is_variant_type_is_pattern_v<Pattern>>> {
      static constexpr bool
          value = is_simple_variant_type_is_pattern_v<Pattern>
                  || is_variant_direct_ref_bind_pattern_v<Pattern>;
      static constexpr bool
          binds = is_variant_direct_ref_bind_pattern_v<Pattern>;
      static constexpr std::size_t index = std::decay_t<
          Pattern>::template alt_index<Subject>();
    };

    template <typename Pattern, typename Subject>
    struct variant_alt_only_pattern<
        Pattern,
        Subject,
        std::enable_if_t<
            is_simple_variant_type_alt_pattern_v<Pattern>
            || is_variant_type_alt_pattern_v<Pattern>>> {
      static constexpr bool
          value = is_simple_variant_type_alt_pattern_v<Pattern>
                  || is_variant_direct_ref_bind_pattern_v<Pattern>;
      static constexpr bool
          binds = is_variant_direct_ref_bind_pattern_v<Pattern>;
      static constexpr std::size_t index = variant_type_alt_index<
          std::decay_t<Pattern>>::value;
    };

    template <typename Pattern, typename Subject>
    struct variant_alt_only_pattern<
        Pattern,
        Subject,
        std::enable_if_t<is_binding_as_pattern_v<Pattern>>> {
      using sub_t = typename binding_as_sub_pattern<
          std::decay_t<Pattern>>::type;
      static constexpr bool
          value = is_simple_variant_type_is_pattern_v<sub_t>
                  || is_simple_variant_type_alt_pattern_v<sub_t>;
      // `$(is<T>)` and `$(alt<I>)` both bind the alternative.
      static constexpr bool binds = value;
      static constexpr std::size_t
          index = variant_alt_only_pattern<sub_t, Subject>::index;
    };

    // The pattern whose match decides the case: the inner pattern of
    // a guarded pattern, otherwise the pattern itself.
    template <typename Pattern, bool = is_guarded_pattern_v<Pattern>>
    struct variant_tested_pattern {
      using type = Pattern;
    };

    template <typename Pattern>
    struct variant_tested_pattern<Pattern, true> {
      using type = std::decay_t<
          typename guarded_inner_pattern<Pattern>::type>;
    };

    template <typename Pattern>
    using variant_tested_pattern_t = typename variant_tested_pattern<
        std::decay_t<Pattern>>::type;

    // Reference to alternative `ActiveIndex`, which the caller knows
    // is active. Unlike `std::get`, this has no `bad_variant_access`
    // path, and stating that the pointer is not null lets the
    // optimizer drop the test of the stored index that `get_if`
    // makes.
    template <std::size_t ActiveIndex, typename Subject>
    PTN_DETAIL_ALWAYS_INLINE constexpr decltype(auto)
    variant_known_alt_get(Subject &subject) {
      auto *alt = std::get_if<ActiveIndex>(&subject);
#if defined(__GNUC__) || defined(__clang__)
      if (alt == nullptr) {
        __builtin_unreachable();
      }
#elif defined(_MSC_VER)
      if (alt == nullptr) {
        __assume(0);
      }
#endif
      return (*alt);
    }

    // Evaluates one case of the bucket for alternative
    // `ActiveIndex`, which the index switch has already selected. A
    // pattern that only tests the alternative is decided at compile
    // time: it matches if it names `ActiveIndex` and rejects
    // otherwise, so only its guard, if any, runs. A binding is taken
    // without testing the index again. The guard and the handler
    // receive the same one-element tuple as on the general path.
    // Other patterns are evaluated in full.
    template <std::size_t ActiveIndex,
              typename Result,
              typename Case,
              typename Subject,
              typename OnMiss>
    PTN_DETAIL_ALWAYS_INLINE constexpr Result
    eval_variant_case_for_known_alt(Case    &current_case,
                                    Subject &subject,
                                    OnMiss &&on_miss) {
      using case_t    = std::remove_reference_t<Case>;
      using pattern_t = std::decay_t<traits::case_pattern_t<case_t>>;
      using subject_t = std::remove_cv_t<Subject>;

      constexpr bool
          guarded      = detail::is_guarded_pattern_v<pattern_t>;
      using alt_only_t = variant_alt_only_pattern<
          variant_tested_pattern_t<pattern_t>,
          subject_t>;

      if constexpr (!alt_only_t::value) {
        return eval_typed_case_or_else<Result>(
            current_case, subject, std::forward<OnMiss>(on_miss));
      }
      else if constexpr (alt_only_t::index != ActiveIndex) {
        return std::forward<OnMiss>(on_miss)();
      }
      else if constexpr (!alt_only_t::binds) {
        if constexpr (guarded) {
          return eval_typed_case_or_else<Result>(
              current_case, subject, std::forward<OnMiss>(on_miss));
        }
        else {
          return invoke_typed_case_handler<Result>(current_case,
                                                   subject);
        }
      }
      else {
        auto bound = std::forward_as_tuple(
            variant_known_alt_get<ActiveIndex>(subject));
        if constexpr (guarded) {
          if (!guard_predicate_holds(current_case.pattern, bound)) {
            return std::forward<OnMiss>(on_miss)();
          }
        }
        return static_cast<Result>(
            detail::invoke_from_tuple(current_case.handler, bound));
      }
    }

    template <std::size_t CaseIndex,
              typename Result,
              typename CasesTuple>
    PTN_DETAIL_ALWAYS_INLINE constexpr Result
    invoke_variant_simple_case_entry(CasesTuple &&cases) {
      return invoke_direct_case_entry<CaseIndex, Result>(
          std::forward<CasesTuple>(cases));
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
          constexpr typename Metadata::key_t
              value = static_literal_case_value<pattern_t,
                                                Subject>::value;
          constexpr std::size_t offset = static_cast<std::size_t>(
              value - Metadata::min_value);

          if (table[offset] == Metadata::k_invalid_case_index) {
            table[offset] = static_cast<
                typename Metadata::case_index_t>(I);
          }
        }

        fill_static_literal_case_index_table<Metadata,
                                             Subject,
                                             CasesTuple,
                                             I + 1,
                                             N>(table);
      }
    }

    template <typename Metadata,
              typename Subject,
              typename CasesTuple>
    constexpr auto make_static_literal_case_index_table() {
      std::array<typename Metadata::case_index_t,
                 Metadata::range_size>
          table{};

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

      static constexpr auto
          case_index_table = make_static_literal_case_index_table<
              metadata_t,
              Subject,
              CasesTuple>();
    };

    // How dense literal dispatch runs the case it selected. Borrowed
    // subjects call the handler directly; owned subjects supply
    // their own entry so the handler sees the owned value
    // categories.
    //
    // `runner` returns a generic lambda that runs case `I` when
    // called with `std::integral_constant<std::size_t, I>`. Its
    // instantiations, one per case, have short debug names; a
    // function template over the case index would repeat the case
    // pack's type in each of them.
    struct direct_literal_case_entry {
      template <typename Result,
                typename Subject,
                typename CasesTuple>
      PTN_DETAIL_ALWAYS_INLINE static constexpr auto
      runner(Subject &, CasesTuple &&cases) {
        auto *pack = &cases;
        return
            [pack](auto case_index) PTN_DETAIL_LAMBDA_ALWAYS_INLINE
            -> Result {
              constexpr std::size_t I = decltype(case_index)::value;
              auto &&current = ptn::core::dsl::detail::case_get<I>(
                  std::forward<CasesTuple>(*pack));
              if constexpr (core::dsl::detail::is_value_handler_v<
                                decltype(current.handler)>) {
                return static_cast<Result>(current.handler.value);
              }
              else {
                return static_cast<Result>(current.handler());
              }
            };
      }
    };

    // Unsigned type of the key's width, in which a dense literal
    // plan computes the subject's offset. `bool` has no
    // std::make_unsigned.
    template <typename Key, bool = std::is_same_v<Key, bool>>
    struct static_literal_offset {
      using type = std::make_unsigned_t<Key>;
    };

    template <typename Key>
    struct static_literal_offset<Key, true> {
      using type = unsigned char;
    };

    template <typename Key>
    using static_literal_offset_t = typename static_literal_offset<
        Key>::type;

    // Dense literal dispatch: the subject's offset from the smallest
    // literal indexes a switch with one arm per offset in the span.
    //
    // The switch is the only range check. A value below the span
    // wraps around to a large offset, and any offset past the span
    // takes the switch's default, like the default of the same
    // `switch` written by hand. A separate `offset < range_size`
    // test in front of it made GCC split the first offset off from
    // the others: a 64-case identity mapping compiled to two
    // compare-and-branch pairs instead of one compare and a `cmov`.
    template <typename Plan,
              typename Result,
              typename Entry,
              typename Subject,
              typename CasesTuple,
              typename Otherwise>
    PTN_DETAIL_ALWAYS_INLINE Result
    eval_cases_impl_static_literal_dispatch(
        Subject     &subject,
        CasesTuple &&cases,
        Otherwise  &&otherwise_handler) {
      static_assert(Plan::range_size
                        <= k_static_literal_dense_dispatch_max_span,
                    "static literal dense dispatch span exceeds "
                    "configured limit");
      static_assert(Plan::range_size <= k_index_switch_max_arms);

      using key_t    = typename Plan::key_t;
      using offset_t = static_literal_offset_t<key_t>;

      // Subtract in the key's own unsigned width. The difference
      // wraps modulo 2^bits, so a value outside the span still lands
      // outside it. A `size_t` offset made GCC sign-extend the key
      // and lower `val<1>..val<64>` mapping to themselves with a
      // branch where the `switch` written by hand gets a `cmov`.
      const auto offset = static_cast<std::size_t>(
          static_cast<offset_t>(
              static_cast<offset_t>(static_cast<key_t>(subject))
              - static_cast<offset_t>(Plan::min_value)));

      using cache_t = static_literal_dispatch_cache<
          Plan,
          Subject,
          std::remove_reference_t<CasesTuple>>;
      auto run_case = Entry::template runner<Result>(
          subject, std::forward<CasesTuple>(cases));

      // The default case, or the otherwise handler without one.
      auto miss = [&]() PTN_DETAIL_LAMBDA_ALWAYS_INLINE -> Result {
        if constexpr (Plan::has_default_case) {
          return run_case(std::integral_constant<
                          std::size_t,
                          static_cast<std::size_t>(
                              Plan::default_case_index)>{});
        }
        else {
          return invoke_otherwise_typed<Result>(
              subject, std::forward<Otherwise>(otherwise_handler));
        }
      };
      // The case at an offset in the span, or `miss` for a gap.
      auto hit = [&](auto slot)
                     PTN_DETAIL_LAMBDA_ALWAYS_INLINE -> Result {
        constexpr auto case_index = cache_t::case_index_table
            [decltype(slot)::value];
        if constexpr (case_index != Plan::k_invalid_case_index) {
          return run_case(std::integral_constant<
                          std::size_t,
                          static_cast<std::size_t>(case_index)>{});
        }
        else {
          return miss();
        }
      };
      return index_visit<Plan::range_size, Result>(
          offset, hit, miss);
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
        auto &current_case = ptn::core::dsl::detail::case_get<I>(
            cases);
        return eval_typed_case_or_else<Result>(
            current_case, subject, [&]() -> Result {
              return eval_cases_impl_typed<I + 1, Result>(
                  subject,
                  cases,
                  std::forward<Otherwise>(otherwise_handler));
            });
      }
    }

    // Index of the first `I` for which `accepts` holds, or the
    // number of indices if none does. The indices are tested in
    // order and the test stops at the first one that holds.
    //
    // A literal plan selects its case this way, then runs it through
    // index_visit. Selecting an index and running the case are kept
    // apart: every branch of this chain ends in the same place with
    // a constant, which is the shape compilers turn into a jump or
    // lookup table. A chain that returns from each case directly
    // does not have it once inlined, and GCC keeps it as a series of
    // comparisons.
    template <typename Accepts, std::size_t... I>
    PTN_DETAIL_ALWAYS_INLINE constexpr std::size_t
    select_first_index(Accepts &accepts, std::index_sequence<I...>) {
      std::size_t selected = sizeof...(I);
      static_cast<void>(
          ((accepts(std::integral_constant<std::size_t, I>{})
            && (selected = I, true))
           || ...));
      return selected;
    }

    template <typename Result,
              typename Subject,
              typename CasesTuple,
              typename Otherwise>
    PTN_DETAIL_ALWAYS_INLINE constexpr Result
    eval_cases_impl_literal_simple_dispatch(
        Subject    &subject,
        CasesTuple &cases,
        Otherwise &&otherwise_handler) {
      constexpr std::size_t case_count = std::tuple_size_v<
          std::remove_cv_t<CasesTuple>>;
      if constexpr (case_count == 0) {
        return invoke_otherwise_typed<Result>(
            subject, std::forward<Otherwise>(otherwise_handler));
      }
      else if constexpr (case_count <= k_index_switch_max_arms) {
        using tuple_t = std::remove_cv_t<CasesTuple>;
        // A `lit(...)` case compares; the trailing `_` accepts.
        auto accepts = [&](auto i) PTN_DETAIL_LAMBDA_ALWAYS_INLINE {
          constexpr std::size_t I = decltype(i)::value;
          if constexpr (is_wildcard_pattern_v<traits::case_pattern_t<
                            std::tuple_element_t<I, tuple_t>>>) {
            return true;
          }
          else {
            return ptn::core::dsl::detail::case_get<I>(cases)
                .pattern.match(std::as_const(subject));
          }
        };
        // The selected case binds nothing.
        auto hit = [&](auto i)
                       PTN_DETAIL_LAMBDA_ALWAYS_INLINE -> Result {
          return static_cast<Result>(
              ptn::core::dsl::detail::case_get<decltype(i)::value>(
                  cases)
                  .handler());
        };
        auto miss = [&]() PTN_DETAIL_LAMBDA_ALWAYS_INLINE -> Result {
          return invoke_otherwise_typed<Result>(
              subject, std::forward<Otherwise>(otherwise_handler));
        };
        return index_visit<case_count, Result>(
            select_first_index(
                accepts, std::make_index_sequence<case_count>{}),
            hit,
            miss);
      }
      else {
        return eval_cases_impl_typed<0, Result>(
            subject,
            cases,
            std::forward<Otherwise>(otherwise_handler));
      }
    }

    // Source-order evaluation for a variant that is
    // valueless_by_exception(). It has no active alternative, so a
    // pattern that only tests the alternative rejects it, guarded or
    // not, and is skipped at compile time. Other cases are evaluated
    // in full.
    template <std::size_t I,
              typename Result,
              typename Subject,
              typename CasesTuple,
              typename Otherwise,
              std::size_t N = std::tuple_size_v<
                  std::remove_reference_t<CasesTuple>>>
    constexpr Result
    eval_cases_impl_typed_valueless(Subject    &subject,
                                    CasesTuple &cases,
                                    Otherwise &&otherwise_handler) {
      if constexpr (I >= N) {
        return invoke_otherwise_typed<Result>(
            subject, std::forward<Otherwise>(otherwise_handler));
      }
      else {
        auto &current_case = ptn::core::dsl::detail::case_get<I>(
            cases);
        using pattern_t = std::decay_t<traits::case_pattern_t<
            std::remove_reference_t<decltype(current_case)>>>;
        using subject_t = std::remove_cv_t<Subject>;

        if constexpr (variant_alt_only_pattern<
                          variant_tested_pattern_t<pattern_t>,
                          subject_t>::value) {
          return eval_cases_impl_typed_valueless<I + 1, Result>(
              subject,
              cases,
              std::forward<Otherwise>(otherwise_handler));
        }
        else {
          return eval_typed_case_or_else<Result>(
              current_case, subject, [&]() -> Result {
                return eval_cases_impl_typed_valueless<I + 1,
                                                       Result>(
                    subject,
                    cases,
                    std::forward<Otherwise>(otherwise_handler));
              });
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
        auto &current_case = ptn::core::dsl::detail::case_get<I>(
            cases);
        using case_t = std::remove_reference_t<
            decltype(current_case)>;
        using pattern_t = traits::case_pattern_t<case_t>;

        if constexpr (variant_pattern_maybe_matches_alt_index<
                          pattern_t,
                          Subject,
                          ActiveIndex>()) {
          return eval_variant_case_for_known_alt<
              ActiveIndex,
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
    PTN_DETAIL_ALWAYS_INLINE constexpr Result
    eval_cases_impl_variant_dispatch_for_alt_hit(
        Subject    &subject,
        CasesTuple &cases,
        Otherwise &&otherwise_handler) {
      if constexpr (Plan::kind
                    == dispatch_plan_kind::variant_simple) {
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
                         == dispatch_plan_kind::
                             variant_alt_bucketed) {
        using bucket_t = typename Plan::template case_bucket<
            AltIndex>;

        if constexpr (bucket_t::has_any) {
          // Replays only the residual case slice selected by the
          // planner for this active alternative. This preserves
          // first-match semantics without scanning unrelated cases.
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

    // Arm of the variant index switch: runs the cases the plan
    // assigned to one alternative. A variant that is
    // valueless_by_exception() reports an index past the last
    // alternative and lands in `miss`.
    template <typename Plan>
    struct variant_alt_arm {
      template <std::size_t AltIndex,
                typename Result,
                typename Subject,
                typename CasesTuple,
                typename Otherwise>
      PTN_DETAIL_ALWAYS_INLINE static constexpr Result
      hit(Subject    &subject,
          CasesTuple &cases,
          Otherwise &&otherwise_handler) {
        return eval_cases_impl_variant_dispatch_for_alt_hit<Plan,
                                                            AltIndex,
                                                            Result>(
            subject,
            cases,
            std::forward<Otherwise>(otherwise_handler));
      }

      template <typename Result,
                typename Subject,
                typename CasesTuple,
                typename Otherwise>
      PTN_DETAIL_ALWAYS_INLINE static constexpr Result
      miss(Subject    &subject,
           CasesTuple &cases,
           Otherwise &&otherwise_handler) {
        using tuple_t = std::remove_reference_t<CasesTuple>;
        constexpr std::size_t
            case_count = std::tuple_size_v<tuple_t>;

        if constexpr (Plan::kind
                          == dispatch_plan_kind::variant_simple
                      && tuple_last_case_is_wildcard<
                          tuple_t,
                          case_count>::value) {
          // Every other case of a simple plan tests the active
          // alternative, so the trailing wildcard is the only case
          // that can match.
          return invoke_direct_case_entry<case_count - 1, Result>(
              cases);
        }
        else {
          // Evaluate in source order. Patterns that only test the
          // alternative reject a valueless variant, so they are
          // skipped without a test; the fallback runs.
          return eval_cases_impl_typed_valueless<0, Result>(
              subject,
              cases,
              std::forward<Otherwise>(otherwise_handler));
        }
      }
    };

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
        return eval_cases_impl_variant_dispatch_for_alt_hit<Plan,
                                                            AltIndex,
                                                            Result>(
            subject, cases, otherwise_handler);
      }
    };

    // Builds a dense cold-path trampoline table from the plan's
    // compact map. Only alternatives that survive planning receive a
    // slot.
    template <typename Plan,
              typename Result,
              typename Subject,
              typename CasesTuple,
              typename OtherwiseHandler,
              std::size_t UsedAltCount,
              std::size_t... AltIndex>
    constexpr auto make_variant_dispatch_compact_table(
        std::index_sequence<AltIndex...>) {
      using dispatch_fn_t = Result (*)(
          Subject &, CasesTuple &, OtherwiseHandler &);
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

    // Caches the trampoline tables for variants with more
    // alternatives than the index switch handles.
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
    PTN_DETAIL_NOINLINE inline Result
    eval_cases_impl_variant_dispatch_cold(
        std::size_t active_index,
        Subject    &subject,
        CasesTuple &cases,
        Otherwise &&otherwise_handler) {
      using otherwise_handler_t = std::remove_reference_t<Otherwise>;
      using cache_t             = variant_dispatch_cache<Plan,
                                                         Result,
                                                         Subject,
                                                         CasesTuple,
                                                         otherwise_handler_t>;

      // First maps the full variant index to the compact trampoline
      // slot.
      const auto compact_index = cache_t::compact_alt_index_map
          [active_index];
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
    PTN_DETAIL_ALWAYS_INLINE constexpr Result
    eval_cases_impl_variant_dispatch(std::size_t active_index,
                                     Subject    &subject,
                                     CasesTuple &cases,
                                     Otherwise &&otherwise_handler) {
      if constexpr (Plan::alt_count <= k_index_switch_max_arms) {
        // The active alternative is already a dense index, so this
        // is the switch a hand-written visitor would use.
        auto hit = [&](auto alt)
                       PTN_DETAIL_LAMBDA_ALWAYS_INLINE -> Result {
          return eval_cases_impl_variant_dispatch_for_alt_hit<
              Plan,
              decltype(alt)::value,
              Result>(subject,
                      cases,
                      std::forward<Otherwise>(otherwise_handler));
        };
        auto miss = [&]() PTN_DETAIL_LAMBDA_ALWAYS_INLINE -> Result {
          return variant_alt_arm<Plan>::template miss<Result>(
              subject,
              cases,
              std::forward<Otherwise>(otherwise_handler));
        };
        return index_visit<Plan::alt_count, Result>(
            active_index, hit, miss);
      }
      else {
        if (active_index >= Plan::alt_count) {
          return eval_cases_impl_typed<0, Result>(
              subject,
              cases,
              std::forward<Otherwise>(otherwise_handler));
        }
        return eval_cases_impl_variant_dispatch_cold<Plan, Result>(
            active_index,
            subject,
            cases,
            std::forward<Otherwise>(otherwise_handler));
      }
    }

    // Executes a precomputed dispatch plan.
    // The planner owns dispatch selection; evaluator code here only
    // maps the selected shape to its runtime entry point.
    template <typename Plan,
              typename Result,
              typename Subject,
              typename CasesTuple,
              typename Otherwise>
    PTN_DETAIL_ALWAYS_INLINE constexpr Result
    eval_cases_with_dispatch_plan(Subject     &subject,
                                  CasesTuple &&cases,
                                  Otherwise  &&otherwise_handler) {
      if constexpr (Plan::kind
                        == dispatch_plan_kind::static_literal_dense
                    || Plan::kind
                           == dispatch_plan_kind::
                               literal_runtime_dense) {
        return eval_cases_impl_static_literal_dispatch<
            Plan,
            Result,
            direct_literal_case_entry>(
            subject,
            std::forward<CasesTuple>(cases),
            std::forward<Otherwise>(otherwise_handler));
      }
      else {
        auto &cases_ref = cases;

        if constexpr (Plan::kind
                      == dispatch_plan_kind::literal_linear) {
          return eval_cases_impl_literal_simple_dispatch<Result>(
              subject,
              cases_ref,
              std::forward<Otherwise>(otherwise_handler));
        }
        else if constexpr (Plan::kind
                           == dispatch_plan_kind::variant_simple) {
          const std::size_t active_index = subject.index();
          return eval_cases_impl_variant_dispatch<Plan, Result>(
              active_index,
              subject,
              cases_ref,
              std::forward<Otherwise>(otherwise_handler));
        }
        else if constexpr (Plan::kind
                           == dispatch_plan_kind::
                               variant_alt_bucketed) {
          const std::size_t active_index = subject.index();
          return eval_cases_impl_variant_dispatch<Plan, Result>(
              active_index,
              subject,
              cases_ref,
              std::forward<Otherwise>(otherwise_handler));
        }
        else {
          return eval_cases_impl_typed<0, Result>(
              subject,
              cases_ref,
              std::forward<Otherwise>(otherwise_handler));
        }
      }
    }
  } // namespace detail

  template <typename Result,
            typename Subject,
            typename CasesTuple,
            typename Otherwise>
  PTN_DETAIL_ALWAYS_INLINE constexpr Result
  eval_cases_typed(Subject     &subject,
                   CasesTuple &&cases,
                   Otherwise  &&otherwise_handler) {
    using dispatch_plan_t = detail::
        dispatch_plan<Subject, std::remove_reference_t<CasesTuple>>;

    return detail::eval_cases_with_dispatch_plan<dispatch_plan_t,
                                                 Result>(
        subject,
        std::forward<CasesTuple>(cases),
        std::forward<Otherwise>(otherwise_handler));
  }
} // namespace ptn::core::common

#undef PTN_DETAIL_NOINLINE
