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
    static constexpr decltype(auto)
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
    static constexpr decltype(auto)
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
    static constexpr auto member(S &subject) {
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
    static constexpr decltype(auto)
    visit(P &pattern, S &subject, Hit &&hit, Miss &&) {
      // Structural patterns always match; instantiate their member
      // validation before extracting the bindings.
      static_cast<void>(pattern.match(std::as_const(subject)));
      return std::forward<Hit>(hit)(
          std::tuple_cat(member<Ms>(subject)...));
    }
  };

  template <typename Tuple, std::size_t... I>
  constexpr auto observe_bindings(Tuple &bound,
                                  std::index_sequence<I...>) {
    return std::forward_as_tuple(
        std::as_const(std::get<I>(bound))...);
  }

  template <typename P, typename Bound, std::size_t... I>
  constexpr bool owned_guard_holds(P     &pattern,
                                   Bound &observed,
                                   std::index_sequence<I...>) {
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
    static constexpr decltype(auto)
    visit(P &pattern, S &subject, Hit &&hit, Miss &&miss) {
      return owned_pattern<Inner>::visit(
          pattern.inner,
          subject,
          [&](auto &&bound) -> decltype(auto) {
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
    static constexpr decltype(auto)
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
    static constexpr decltype(auto)
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
    static constexpr decltype(auto)
    visit(P &pattern, S &subject, Hit &&hit, Miss &&miss) {
      return owned_variant_pattern<selector, Sub>::visit(
          pattern,
          subject,
          std::forward<Hit>(hit),
          std::forward<Miss>(miss));
    }
  };

  template <typename Bound, std::size_t... I>
  constexpr auto forward_bindings(Bound &&bound,
                                  std::index_sequence<I...>) {
    return std::forward_as_tuple(
        forward_bound_element<I, Bound>(bound)...);
  }

  template <typename Tag, typename Sub>
  struct owned_pattern<pat::detail::binding_as_pattern<Tag, Sub>> {
    template <typename S, typename Bound>
    static constexpr auto combine(S &subject, Bound &&bound) {
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
    static constexpr decltype(auto)
    visit(P &pattern, S &subject, Hit &&hit, Miss &&miss) {
      return owned_pattern<Sub>::visit(
          pattern.subpattern,
          subject,
          [&](auto &&bound) -> decltype(auto) {
            return std::forward<Hit>(hit)(combine(
                subject, std::forward<decltype(bound)>(bound)));
          },
          std::forward<Miss>(miss));
    }
  };

  template <typename P, typename S>
  constexpr bool owned_matches(P &pattern, S &subject) {
    return owned_pattern<std::decay_t<P>>::visit(
        pattern,
        subject,
        [](auto &&) { return true; },
        [] { return false; });
  }

  template <bool All>
  struct owned_combinator {
    template <typename S>
    using bindings = std::tuple<>;
    template <typename P, typename S, typename Hit, typename Miss>
    static constexpr decltype(auto)
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
    static constexpr decltype(auto)
    visit(Pattern &pattern, S &subject, Hit &&hit, Miss &&miss) {
      if (!owned_matches(pattern.sub, subject)) {
        return std::forward<Hit>(hit)(std::tuple<>{});
      }
      return std::forward<Miss>(miss)();
    }
  };

  template <typename Case, typename Bound, std::size_t... I>
  constexpr decltype(auto) invoke_owned_handler(
      Case &&current, Bound &&bound, std::index_sequence<I...>) {
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
              decltype(std::get<I>(std::declval<Cases>())),
              S>...>;

  template <typename Result,
            std::size_t I = 0,
            typename S,
            typename Cases>
  constexpr Result eval_owned_cases(S &subject, Cases &&cases) {
    if constexpr (I
                  == std::tuple_size_v<
                      std::remove_reference_t<Cases>>) {
      return unreachable_result<Result>();
    }
    else {
      auto &current = std::get<I>(cases);
      using pattern = traits::case_pattern_t<
          std::decay_t<decltype(current)>>;
      return owned_pattern<pattern>::visit(
          current.pattern,
          subject,
          [&](auto &&bound) -> Result {
            return static_cast<Result>(invoke_owned_handler(
                std::get<I>(std::forward<Cases>(cases)),
                std::forward<decltype(bound)>(bound),
                std::make_index_sequence<std::tuple_size_v<
                    std::remove_reference_t<decltype(bound)>>>{}));
          },
          [&]() -> Result {
            return eval_owned_cases<Result, I + 1>(
                subject, std::forward<Cases>(cases));
          });
    }
  }
} // namespace ptn::core::common::detail
