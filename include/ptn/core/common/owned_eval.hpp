#pragma once

#include <tuple>
#include <type_traits>
#include <utility>

#include "ptn/core/common/eval.hpp"
#include "ptn/pattern/bind.hpp"
#include "ptn/pattern/type.hpp"
#include "ptn/pattern/combinator.hpp"
#include "ptn/pattern/negation.hpp"

namespace ptn::core::common::detail {

  // A successful visit keeps its binding storage alive until the
  // continuation returns. Only that continuation may consume it.
  // Custom patterns retain their const-subject binding contract;
  // values they materialize are owned by this invocation.
  template <typename Pattern>
  struct owned_pattern {
    template <typename Subject>
    using bindings = decltype(std::declval<const Pattern &>().bind(
        std::declval<const Subject &>()));

    template <typename P, typename S, typename Hit, typename Miss>
    PTN_DETAIL_ALWAYS_INLINE static constexpr decltype(auto)
    visit(P &pattern, S &subject, Hit &&hit, Miss &&miss) {
      if (std::as_const(pattern).match(std::as_const(subject))) {
        auto &&bound = std::as_const(pattern).bind(
            std::as_const(subject));
        return std::forward<Hit>(hit)(
            std::forward<decltype(bound)>(bound));
      }
      return std::forward<Miss>(miss)();
    }
  };

  template <typename Pattern, typename Subject>
  using owned_bindings_t = typename owned_pattern<
      Pattern>::template bindings<Subject>;

  struct owned_subject_binding {
    template <typename S>
    using bindings = std::tuple<S &&>;

    template <typename P, typename S, typename Hit, typename Miss>
    PTN_DETAIL_ALWAYS_INLINE static constexpr decltype(auto)
    visit(P &, S &subject, Hit &&hit, Miss &&) {
      return std::forward<Hit>(hit)(
          std::forward_as_tuple(std::move(subject)));
    }
  };

  template <>
  struct owned_pattern<pat::bind_factory> : owned_subject_binding {};
  template <>
  struct owned_pattern<pat::detail::binding_pattern>
      : owned_subject_binding {};

  template <auto... Ms>
  struct owned_pattern<pat::detail::structural_bind_pattern<
      pat::detail::has_pattern<Ms...>>> {
    template <auto M, typename S>
    PTN_DETAIL_ALWAYS_INLINE static constexpr auto
    member(S &subject) {
      if constexpr (std::is_null_pointer_v<decltype(M)>) {
        return std::tuple<>{};
      }
      else {
        return std::forward_as_tuple(std::move(subject).*M);
      }
    }

    template <typename S>
    using bindings = decltype(std::tuple_cat(
        member<Ms>(std::declval<S &>())...));

    template <typename P, typename S, typename Hit, typename Miss>
    PTN_DETAIL_ALWAYS_INLINE static constexpr decltype(auto)
    visit(P &pattern, S &subject, Hit &&hit, Miss &&) {
      // Structural patterns always match; instantiate their member
      // validation before extracting the bindings.
      static_cast<void>(pattern.match(std::as_const(subject)));
      return std::forward<Hit>(hit)(
          std::tuple_cat(member<Ms>(subject)...));
    }
  };

  template <typename Tuple, std::size_t... I>
  PTN_DETAIL_ALWAYS_INLINE constexpr auto
  observe_bindings(Tuple &bound, std::index_sequence<I...>) {
    return std::forward_as_tuple(
        std::as_const(std::get<I>(bound))...);
  }

  template <typename P, typename Bound, std::size_t... I>
  PTN_DETAIL_ALWAYS_INLINE constexpr bool owned_guard_holds(
      P &pattern, Bound &observed, std::index_sequence<I...>) {
    if constexpr (pat::traits::is_tuple_guard_predicate_v<
                      decltype(pattern.pred)>) {
      return guard_predicate_holds(pattern, observed);
    }
    else {
      constexpr bool valid = std::is_invocable_v<
          decltype(pattern.pred) &,
          decltype(std::get<I>(observed))...>;
      static_assert(
          valid,
          "[Patternia.guard]: guards on owned subjects must "
          "accept read-only bindings. Consume values in the "
          "handler.");
      if constexpr (valid)
        return guard_predicate_holds(pattern, observed);
      else
        return false;
    }
  }

  template <typename Inner, typename Pred>
  struct owned_pattern<pat::mod::guarded_pattern<Inner, Pred>> {
    template <typename S>
    using bindings = owned_bindings_t<Inner, S>;

    template <typename P, typename S, typename Hit, typename Miss>
    PTN_DETAIL_ALWAYS_INLINE static constexpr decltype(auto)
    visit(P &pattern, S &subject, Hit &&hit, Miss &&miss) {
      return owned_pattern<Inner>::visit(
          pattern.inner,
          subject,
          [&](auto &&bound) PTN_DETAIL_LAMBDA_ALWAYS_INLINE
          -> decltype(auto) {
            auto observed = observe_bindings(
                bound,
                std::make_index_sequence<std::tuple_size_v<
                    std::remove_reference_t<decltype(bound)>>>{});
            if (owned_guard_holds(
                    pattern,
                    observed,
                    std::make_index_sequence<
                        std::tuple_size_v<decltype(observed)>>{})) {
              return std::forward<Hit>(hit)(
                  std::forward<decltype(bound)>(bound));
            }
            // Rejected bindings remain alive while later cases run.
            // Flattening this continuation chain is performance
            // work.
            return std::forward<Miss>(miss)();
          },
          std::forward<Miss>(miss));
    }
  };

  // Type wrappers recurse into the selected alternative, so a nested
  // guard and its handler share the same materialized bindings.
  template <typename Pattern, typename Sub>
  struct owned_variant_pattern {
    template <typename S>
    using alternative = std::remove_reference_t<
        decltype(std::get<Pattern::template alt_index<S>()>(
            std::declval<S &>()))>;

    template <
        typename S,
        bool Empty = std::is_same_v<Sub, pat::detail::no_subpattern>>
    struct binding_type {
      using type = owned_bindings_t<Sub, alternative<S>>;
    };
    template <typename S>
    struct binding_type<S, true> {
      using type = std::tuple<>;
    };
    template <typename S>
    using bindings = typename binding_type<S>::type;

    template <typename P, typename S, typename Hit, typename Miss>
    PTN_DETAIL_ALWAYS_INLINE static constexpr decltype(auto)
    visit(P &pattern, S &subject, Hit &&hit, Miss &&miss) {
      constexpr auto index = Pattern::template alt_index<S>();
      if (subject.index() != index) {
        return std::forward<Miss>(miss)();
      }
      if constexpr (std::is_same_v<Sub,
                                   pat::detail::no_subpattern>) {
        return std::forward<Hit>(hit)(std::tuple<>{});
      }
      else {
        return owned_pattern<Sub>::visit(pattern.subpattern,
                                         std::get<index>(subject),
                                         std::forward<Hit>(hit),
                                         std::forward<Miss>(miss));
      }
    }
  };

  template <typename T, typename Sub>
  struct owned_pattern<pat::detail::type_is_pattern<T, Sub>> {
    struct selector {
      template <typename S>
      static constexpr std::size_t alt_index() {
        static_assert_variant_alt_unique<
            typename pat::detail::type_is_pattern<T, Sub>::alt_t,
            S>();
        return pat::detail::type_is_pattern<T, Sub>::
            template alt_index<S>();
      }
    };
    template <typename S>
    using bindings = typename owned_variant_pattern<selector, Sub>::
        template bindings<S>;
    template <typename P, typename S, typename Hit, typename Miss>
    PTN_DETAIL_ALWAYS_INLINE static constexpr decltype(auto)
    visit(P &pattern, S &subject, Hit &&hit, Miss &&miss) {
      return owned_variant_pattern<selector, Sub>::visit(
          pattern,
          subject,
          std::forward<Hit>(hit),
          std::forward<Miss>(miss));
    }
  };

  // alt<I> uses the same recursion, with an index-based selector.
  template <std::size_t I, typename Sub>
  struct owned_pattern<pat::detail::type_alt_pattern<I, Sub>> {
    struct selector {
      template <typename S>
      static constexpr std::size_t alt_index() {
        static_assert_variant_alt_index<I, S>();
        return I;
      }
    };
    template <typename S>
    using bindings = typename owned_variant_pattern<selector, Sub>::
        template bindings<S>;
    template <typename P, typename S, typename Hit, typename Miss>
    PTN_DETAIL_ALWAYS_INLINE static constexpr decltype(auto)
    visit(P &pattern, S &subject, Hit &&hit, Miss &&miss) {
      return owned_variant_pattern<selector, Sub>::visit(
          pattern,
          subject,
          std::forward<Hit>(hit),
          std::forward<Miss>(miss));
    }
  };

  template <typename Bound, std::size_t... I>
  PTN_DETAIL_ALWAYS_INLINE constexpr auto
  forward_bindings(Bound &&bound, std::index_sequence<I...>) {
    return std::forward_as_tuple(
        forward_bound_element<I, Bound>(bound)...);
  }

  template <typename Tag, typename Sub>
  struct owned_pattern<pat::detail::binding_as_pattern<Tag, Sub>> {
    template <typename S, typename Bound>
    PTN_DETAIL_ALWAYS_INLINE static constexpr auto
    combine(S &subject, Bound &&bound) {
      if constexpr ((pat::detail::has_alt_index<Sub, S>::value
                     || is_variant_type_alt_pattern_v<Sub>)
                    && std::tuple_size_v<
                           std::remove_reference_t<Bound>>
                           == 0) {
        constexpr auto index = [] {
          if constexpr (pat::detail::has_alt_index<Sub, S>::value)
            return Sub::template alt_index<S>();
          else
            return variant_type_alt_index<Sub>::value;
        }();
        return std::forward_as_tuple(
            std::get<index>(std::move(subject)));
      }
      else {
        return std::tuple_cat(
            std::forward_as_tuple(std::move(subject)),
            forward_bindings(
                std::forward<Bound>(bound),
                std::make_index_sequence<std::tuple_size_v<
                    std::remove_reference_t<Bound>>>{}));
      }
    }
    template <typename S>
    using bindings = decltype(combine(
        std::declval<S &>(),
        std::declval<owned_bindings_t<Sub, S>>()));
    template <typename P, typename S, typename Hit, typename Miss>
    PTN_DETAIL_ALWAYS_INLINE static constexpr decltype(auto)
    visit(P &pattern, S &subject, Hit &&hit, Miss &&miss) {
      return owned_pattern<Sub>::visit(
          pattern.subpattern,
          subject,
          [&](auto &&bound)
              PTN_DETAIL_LAMBDA_ALWAYS_INLINE -> decltype(auto) {
                return std::forward<Hit>(hit)(combine(
                    subject, std::forward<decltype(bound)>(bound)));
              },
          std::forward<Miss>(miss));
    }
  };

  template <typename P, typename S>
  PTN_DETAIL_ALWAYS_INLINE constexpr bool owned_matches(P &pattern,
                                                        S &subject) {
    return owned_pattern<std::decay_t<P>>::visit(
        pattern,
        subject,
        [](auto &&) PTN_DETAIL_LAMBDA_ALWAYS_INLINE { return true; },
        []() PTN_DETAIL_LAMBDA_ALWAYS_INLINE { return false; });
  }

  template <bool All>
  struct owned_combinator {
    template <typename S>
    using bindings = std::tuple<>;
    template <typename P, typename S, typename Hit, typename Miss>
    PTN_DETAIL_ALWAYS_INLINE static constexpr decltype(auto)
    visit(P &pattern, S &subject, Hit &&hit, Miss &&miss) {
      const bool matched = std::apply(
          [&](auto &...parts) {
            if constexpr (All)
              return (owned_matches(parts, subject) && ...);
            else
              return (owned_matches(parts, subject) || ...);
          },
          pattern.patterns);
      if (matched)
        return std::forward<Hit>(hit)(std::tuple<>{});
      return std::forward<Miss>(miss)();
    }
  };
  template <typename... Ps>
  struct owned_pattern<pat::detail::any_pattern<Ps...>>
      : owned_combinator<false> {};
  template <typename... Ps>
  struct owned_pattern<pat::detail::all_pattern<Ps...>>
      : owned_combinator<true> {};
  template <typename P>
  struct owned_pattern<pat::detail::negation_pattern<P>> {
    template <typename S>
    using bindings = std::tuple<>;
    template <typename Pattern,
              typename S,
              typename Hit,
              typename Miss>
    PTN_DETAIL_ALWAYS_INLINE static constexpr decltype(auto)
    visit(Pattern &pattern, S &subject, Hit &&hit, Miss &&miss) {
      if (!owned_matches(pattern.sub, subject)) {
        return std::forward<Hit>(hit)(std::tuple<>{});
      }
      return std::forward<Miss>(miss)();
    }
  };

  template <typename Case, typename Bound, std::size_t... I>
  PTN_DETAIL_ALWAYS_INLINE constexpr decltype(auto)
  invoke_owned_handler(Case  &&current,
                       Bound &&bound,
                       std::index_sequence<I...>) {
    auto &&handler  = std::forward<Case>(current).handler;
    using handler_t = decltype(handler);
    if constexpr (std::is_invocable_v<
                      handler_t,
                      decltype(forward_bound_element<I, Bound>(
                          bound))...>) {
      return std::invoke(std::forward<handler_t>(handler),
                         forward_bound_element<I, Bound>(bound)...);
    }
    else {
      static_assert(
          std::is_invocable_v<handler_t>,
          "[Patternia.match]: handler cannot accept the owned "
          "bindings. Preserve constness and use an rvalue case "
          "pack for a consuming handler.");
      if constexpr (std::is_invocable_v<handler_t>) {
        return std::invoke(std::forward<handler_t>(handler));
      }
    }
  }

  template <typename Case, typename S>
  using owned_case_bindings_t = owned_bindings_t<
      traits::case_pattern_t<std::decay_t<Case>>,
      S>;
  template <typename Case, typename S>
  using owned_case_result_t = decltype(invoke_owned_handler(
      std::declval<Case>(),
      std::declval<owned_case_bindings_t<Case, S>>(),
      std::make_index_sequence<
          std::tuple_size_v<std::remove_reference_t<
              owned_case_bindings_t<Case, S>>>>{}));

  template <typename Enable, typename... Results>
  struct owned_common_result {
    static constexpr bool valid = false;
  };
  template <typename... Results>
  struct owned_common_result<
      std::void_t<std::common_type_t<Results...>>,
      Results...> {
    // Some standard library/compiler combinations accept a mixed
    // void common_type as an extension. Keep the contract portable.
    static constexpr bool valid = (std::is_void_v<Results> && ...)
                                  || (!std::is_void_v<Results>
                                      && ...);
    using type = std::common_type_t<Results...>;
  };

  template <typename S, typename Cases, std::size_t... I>
  auto owned_result_type(std::index_sequence<I...>)
      -> owned_common_result<
          void,
          owned_case_result_t<
              decltype(ptn::core::dsl::detail::case_get<I>(
                  std::declval<Cases>())),
              S>...>;

  // Case filters for eval_owned_cases_in_order. A filter only skips
  // cases that cannot match, so the first matching case is the same
  // with or without it.
  struct owned_every_case {
    static constexpr bool        knows_alt = false;
    static constexpr std::size_t known_alt = 0;

    template <typename Pattern, typename S>
    static constexpr bool may_match() {
      return true;
    }
  };

  // The cases that can match while alternative `AltIndex` is active:
  // a case whose pattern names another alternative is skipped.
  template <std::size_t AltIndex>
  struct owned_cases_for_alt {
    static constexpr bool        knows_alt = true;
    static constexpr std::size_t known_alt = AltIndex;

    template <typename Pattern, typename S>
    static constexpr bool may_match() {
      return variant_pattern_maybe_matches_alt_index<Pattern,
                                                     S,
                                                     AltIndex>();
    }
  };

  // Owned bindings of a pattern that only tests the alternative,
  // while alternative `AltIndex` is known to be active: none for
  // `is<T>` and `alt<I>`, the alternative as an rvalue for `$(...)`
  // and a direct reference binding, as the general visit binds them.
  template <std::size_t AltIndex, bool Binds, typename S>
  PTN_DETAIL_ALWAYS_INLINE constexpr auto
  owned_known_alt_bindings(S &subject) {
    if constexpr (Binds) {
      return std::forward_as_tuple(
          std::move(variant_known_alt_get<AltIndex>(subject)));
    }
    else {
      return std::tuple<>{};
    }
  }

  // `true` if case pattern `Pattern` only tests the alternative and
  // its known-alternative bindings have the same type as those of
  // the general owned visit, so the case can skip the index test.
  template <std::size_t AltIndex, typename Pattern, typename S>
  constexpr bool owned_case_is_known_alt() {
    using pattern_t  = std::decay_t<Pattern>;
    using tested_t   = variant_tested_pattern_t<pattern_t>;
    using alt_only_t = variant_alt_only_pattern<tested_t,
                                                std::remove_cv_t<S>>;
    if constexpr (!alt_only_t::value) {
      return false;
    }
    else if constexpr (alt_only_t::index != AltIndex) {
      return false;
    }
    else {
      return std::is_same_v<
          decltype(owned_known_alt_bindings<AltIndex,
                                            alt_only_t::binds>(
              std::declval<S &>())),
          owned_bindings_t<tested_t, S>>;
    }
  }

  // `true` if `Filter` knows the active alternative and the case can
  // use it. owned_case_is_known_alt is instantiated only in an arm
  // of the owned variant switch, never for other subjects.
  template <typename Filter,
            typename Pattern,
            typename S,
            bool = Filter::knows_alt>
  struct owned_case_takes_known_alt : std::false_type {};

  template <typename Filter, typename Pattern, typename S>
  struct owned_case_takes_known_alt<Filter, Pattern, S, true>
      : std::bool_constant<owned_case_is_known_alt<Filter::known_alt,
                                                   Pattern,
                                                   S>()> {};

  // Visits a case that owned_case_is_known_alt accepts: binds
  // without testing the index, then runs the guard, if any, on
  // read-only views of the bindings as the general guarded visit
  // does.
  template <std::size_t AltIndex,
            typename P,
            typename S,
            typename Hit,
            typename Miss>
  PTN_DETAIL_ALWAYS_INLINE constexpr decltype(auto)
  owned_visit_known_alt(P     &pattern,
                        S     &subject,
                        Hit  &&hit,
                        Miss &&miss) {
    using pattern_t  = std::decay_t<P>;
    using alt_only_t = variant_alt_only_pattern<
        variant_tested_pattern_t<pattern_t>,
        std::remove_cv_t<S>>;
    auto bound = owned_known_alt_bindings<AltIndex,
                                          alt_only_t::binds>(
        subject);
    if constexpr (is_guarded_pattern_v<pattern_t>) {
      auto observed = observe_bindings(
          bound,
          std::make_index_sequence<
              std::tuple_size_v<decltype(bound)>>{});
      if (!owned_guard_holds(
              pattern,
              observed,
              std::make_index_sequence<
                  std::tuple_size_v<decltype(observed)>>{})) {
        return std::forward<Miss>(miss)();
      }
    }
    return std::forward<Hit>(hit)(std::move(bound));
  }

  // The cases that can match a variant that is
  // valueless_by_exception(): it has no active alternative, so a
  // pattern that only tests the alternative rejects it, guarded or
  // not, and is skipped.
  struct owned_cases_for_valueless {
    static constexpr bool        knows_alt = false;
    static constexpr std::size_t known_alt = 0;

    template <typename Pattern, typename S>
    static constexpr bool may_match() {
      return !variant_alt_only_pattern<
          variant_tested_pattern_t<std::decay_t<Pattern>>,
          std::remove_cv_t<S>>::value;
    }
  };

  template <typename Result,
            std::size_t I   = 0,
            typename Filter = owned_every_case,
            typename S,
            typename Cases>
  PTN_DETAIL_ALWAYS_INLINE constexpr Result
  eval_owned_cases_in_order(S &subject, Cases &&cases) {
    if constexpr (I
                  == std::tuple_size_v<
                      std::remove_reference_t<Cases>>) {
      return unreachable_result<Result>();
    }
    else if constexpr (!Filter::template may_match<
                           traits::case_pattern_t<
                               std::tuple_element_t<
                                   I,
                                   std::remove_reference_t<Cases>>>,
                           S>()) {
      return eval_owned_cases_in_order<Result, I + 1, Filter>(
          subject, std::forward<Cases>(cases));
    }
    else {
      auto &current = ptn::core::dsl::detail::case_get<I>(cases);
      using pattern = traits::case_pattern_t<
          std::decay_t<decltype(current)>>;
      auto on_hit = [&](auto &&bound)
                        PTN_DETAIL_LAMBDA_ALWAYS_INLINE -> Result {
        return static_cast<Result>(invoke_owned_handler(
            ptn::core::dsl::detail::case_get<I>(
                std::forward<Cases>(cases)),
            std::forward<decltype(bound)>(bound),
            std::make_index_sequence<std::tuple_size_v<
                std::remove_reference_t<decltype(bound)>>>{}));
      };
      auto on_miss = [&]()
                         PTN_DETAIL_LAMBDA_ALWAYS_INLINE -> Result {
        return eval_owned_cases_in_order<Result, I + 1, Filter>(
            subject, std::forward<Cases>(cases));
      };
      if constexpr (owned_case_takes_known_alt<Filter, pattern, S>::
                        value) {
        return owned_visit_known_alt<Filter::known_alt>(
            current.pattern, subject, on_hit, on_miss);
      }
      else {
        return owned_pattern<pattern>::visit(
            current.pattern, subject, on_hit, on_miss);
      }
    }
  }

  // Smallest number of alternatives for which an owned variant match
  // switches on the active alternative; see eval_owned_cases.
  constexpr std::size_t k_owned_variant_switch_min_alts = 5;

  [[noreturn]] PTN_DETAIL_ALWAYS_INLINE void
  variant_alt_unreachable() {
#if defined(__GNUC__) || defined(__clang__)
    __builtin_unreachable();
#elif defined(_MSC_VER)
    __assume(0);
#endif
  }

  // Arm of the index switch over the active alternative of an owned
  // variant subject. Each arm evaluates, in source order, only the
  // cases that can match its alternative. A variant that is
  // valueless_by_exception() reports `variant_npos` and lands in
  // `miss`, which skips the cases that only test the alternative.
  struct owned_variant_alt_arm {
    template <std::size_t AltIndex,
              typename Result,
              typename S,
              typename Cases>
    PTN_DETAIL_ALWAYS_INLINE static constexpr Result
    hit(S &subject, Cases &&cases) {
      // The switch has selected this arm on `index()`. Stating it
      // lets the compiler drop the index tests of the cases below
      // and the `bad_variant_access` path of their `std::get`, as
      // long as nothing writes to the subject in between.
      if (subject.index() != AltIndex) {
        variant_alt_unreachable();
      }
      return eval_owned_cases_in_order<
          Result,
          0,
          owned_cases_for_alt<AltIndex>>(subject,
                                         std::forward<Cases>(cases));
    }

    template <typename Result, typename S, typename Cases>
    PTN_DETAIL_ALWAYS_INLINE static constexpr Result
    miss(S &subject, Cases &&cases) {
      return eval_owned_cases_in_order<Result,
                                       0,
                                       owned_cases_for_valueless>(
          subject, std::forward<Cases>(cases));
    }
  };

  // Runs the literal case that dense literal dispatch or the literal
  // chain selected for an owned subject. A literal case binds
  // nothing, so its handler is invoked directly; the trailing
  // fallback goes through the ordinary owned visit so it still
  // receives the subject as an rvalue.
  struct owned_literal_case_entry {
    template <std::size_t CaseIndex,
              typename Result,
              typename S,
              typename Cases>
    PTN_DETAIL_ALWAYS_INLINE static constexpr Result
    invoke(S &subject, Cases &&cases) {
      using pattern = traits::case_pattern_t<
          std::tuple_element_t<CaseIndex,
                               std::remove_reference_t<Cases>>>;
      if constexpr (is_static_literal_pattern_v<pattern>
                    || is_simple_literal_pattern_v<pattern>) {
        return static_cast<Result>(invoke_owned_handler(
            ptn::core::dsl::detail::case_get<CaseIndex>(
                std::forward<Cases>(cases)),
            std::tuple<>{},
            std::index_sequence<>{}));
      }
      else {
        return eval_owned_cases_in_order<Result, CaseIndex>(
            subject, std::forward<Cases>(cases));
      }
    }

    // The same, as the generic lambda dense literal dispatch calls
    // with `std::integral_constant<std::size_t, CaseIndex>`; see
    // direct_literal_case_entry.
    template <typename Result, typename S, typename Cases>
    PTN_DETAIL_ALWAYS_INLINE static constexpr auto
    runner(S &subject, Cases &&cases) {
      auto *owned = &subject;
      auto *pack  = &cases;
      return [owned,
              pack](auto case_index) PTN_DETAIL_LAMBDA_ALWAYS_INLINE
             -> Result {
        constexpr std::size_t I = decltype(case_index)::value;
        using pattern           = traits::case_pattern_t<
                      std::tuple_element_t<I, std::remove_reference_t<Cases>>>;
        if constexpr (is_static_literal_pattern_v<pattern>
                      || is_simple_literal_pattern_v<pattern>) {
          return static_cast<Result>(invoke_owned_handler(
              ptn::core::dsl::detail::case_get<I>(
                  std::forward<Cases>(*pack)),
              std::tuple<>{},
              std::index_sequence<>{}));
        }
        else {
          return eval_owned_cases_in_order<Result, I>(
              *owned, std::forward<Cases>(*pack));
        }
      };
    }
  };

  // Compares an owned subject with a run of runtime `lit(...)`
  // literals, in source order, as the borrowed literal chain does.
  // The cases bind nothing and have no guards, so each test is a
  // plain comparison; the selected handler still runs through the
  // owned entry. Every step is inlined so the case pack stays out of
  // memory.
  template <typename Result,
            std::size_t I = 0,
            typename S,
            typename Cases>
  PTN_DETAIL_ALWAYS_INLINE constexpr Result
  eval_owned_literal_chain(S &subject, Cases &&cases) {
    if constexpr (I
                  == std::tuple_size_v<
                      std::remove_reference_t<Cases>>) {
      return unreachable_result<Result>();
    }
    else {
      using pattern = traits::case_pattern_t<
          std::tuple_element_t<I, std::remove_reference_t<Cases>>>;
      if constexpr (is_simple_literal_pattern_v<pattern>) {
        if (ptn::core::dsl::detail::case_get<I>(cases).pattern.match(
                std::as_const(subject))) {
          return owned_literal_case_entry::template invoke<I,
                                                           Result>(
              subject, std::forward<Cases>(cases));
        }
        return eval_owned_literal_chain<Result, I + 1>(
            subject, std::forward<Cases>(cases));
      }
      else {
        // The fallback: `_` matches everything.
        return owned_literal_case_entry::template invoke<I, Result>(
            subject, std::forward<Cases>(cases));
      }
    }
  }

  // Owned subjects are matched in source order, except where the
  // planner proves that a keyed dispatch selects the same case: a
  // dense run of static literals followed by a fallback. Those cases
  // have no guards and no bindings, so jumping straight to the first
  // case with the subject's value is not observable. A run of
  // runtime literals is still compared in order, but through the
  // same inlined chain as a borrowed subject. A variant subject
  // switches on its active alternative and then runs, in source
  // order, only the cases that can match that alternative; skipping
  // the others is not observable either.
  template <typename Result, typename S, typename Cases>
  PTN_DETAIL_ALWAYS_INLINE constexpr Result
  eval_owned_cases(S &subject, Cases &&cases) {
    using plan = dispatch_plan<S, std::remove_reference_t<Cases>>;
    constexpr bool
        keyed = plan::kind
                    == dispatch_plan_kind::static_literal_dense
                || plan::kind
                       == dispatch_plan_kind::literal_runtime_dense;
    if constexpr (plan::kind == dispatch_plan_kind::literal_linear) {
      return eval_owned_literal_chain<Result>(
          subject, std::forward<Cases>(cases));
    }
    else if constexpr (keyed) {
      if constexpr (plan::has_default_case) {
        return eval_cases_impl_static_literal_dispatch<
            plan,
            Result,
            owned_literal_case_entry>(
            subject, std::forward<Cases>(cases), [] {});
      }
      else {
        return eval_owned_cases_in_order<Result>(
            subject, std::forward<Cases>(cases));
      }
    }
    else if constexpr (plan::kind
                           == dispatch_plan_kind::variant_simple
                       || plan::kind
                              == dispatch_plan_kind::
                                  variant_alt_bucketed) {
      // The borrowed path's variant plan, with the owned evaluation
      // of each case. Up to four alternatives, the cases stay in
      // source order: compilers already merge those few index tests
      // well, and Apple Clang 17 lowers a 4-case switch to a tree of
      // compares where the source-order tests become branchless
      // selects. Past that, GCC kept one compare per alternative in
      // source order (15 for 16 alternatives).
      if constexpr (plan::alt_count
                        >= k_owned_variant_switch_min_alts
                    && plan::alt_count <= k_index_switch_max_arms) {
        auto hit = [&](auto alt)
                       PTN_DETAIL_LAMBDA_ALWAYS_INLINE -> Result {
          return owned_variant_alt_arm::
              template hit<decltype(alt)::value, Result>(
                  subject, std::forward<Cases>(cases));
        };
        auto miss = [&]() PTN_DETAIL_LAMBDA_ALWAYS_INLINE -> Result {
          return owned_variant_alt_arm::template miss<Result>(
              subject, std::forward<Cases>(cases));
        };
        return index_visit<plan::alt_count, Result>(
            subject.index(), hit, miss);
      }
      else {
        return eval_owned_cases_in_order<Result>(
            subject, std::forward<Cases>(cases));
      }
    }
    else {
      return eval_owned_cases_in_order<Result>(
          subject, std::forward<Cases>(cases));
    }
  }
} // namespace ptn::core::common::detail
