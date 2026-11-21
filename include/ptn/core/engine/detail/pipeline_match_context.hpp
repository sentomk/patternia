#pragma once

// Implementation of the pipeline-only match context.
//
// This header contains the internal object returned by
// `match(subject)`. It borrows lvalue subjects and owns rvalue
// subjects until evaluation through `std::move(context) | on(...)`.

#include <tuple>
#include <type_traits>
#include <utility>

#include "ptn/core/common/common_traits.hpp"
#include "ptn/core/common/diagnostics.hpp"
#include "ptn/core/common/inline.hpp"
#include "ptn/core/common/owned_eval.hpp"
#include "ptn/core/dsl/detail/case_expr_impl.hpp"
#include "ptn/core/engine/detail/match_impl.hpp"

namespace ptn::core::engine::detail {

  // Checks for `on<R>(...)`: a handler result `From` must convert to
  // `R`. A reference `R` must bind to the handler's own result, not
  // to a temporary converted from it, which would dangle once the
  // match returns. Type-only, so nothing here runs at compile time
  // but the traits themselves.
  template <typename From, typename To>
  struct is_explicit_result_reference_convertible
      : std::bool_constant<std::is_reference_v<From>
                           && std::is_convertible_v<From, To>
                           && std::is_convertible_v<
                               std::remove_reference_t<From> *,
                               std::remove_reference_t<To> *>> {};

  template <typename From, typename To>
  struct is_explicit_result_convertible
      : std::conditional_t<
            std::is_void_v<To>,
            std::true_type,
            std::conditional_t<
                std::is_reference_v<To>,
                is_explicit_result_reference_convertible<From, To>,
                std::is_convertible<From, To>>> {};

  template <typename From, typename To>
  inline constexpr bool is_explicit_result_convertible_v =
      is_explicit_result_convertible<From, To>::value;

  // One assertion per case, so the compiler names the case index of
  // a handler whose result does not fit.
  template <typename Result, std::size_t CaseIndex, typename From>
  constexpr void static_assert_explicit_result() {
    static_assert(
        is_explicit_result_convertible_v<From, Result>,
        "[Patternia.on]: a handler's result does not convert to the "
        "result type given to on<R>(...). Every handler is checked, "
        "including handlers of cases that are never selected. A "
        "reference R must bind directly to a reference the handler "
        "returns.");
  }

  template <typename Result, typename... From, std::size_t... I>
  constexpr void
  static_assert_explicit_results(std::index_sequence<I...>) {
    (static_assert_explicit_result<Result, I, From>(), ...);
  }

  template <typename Result, typename Tuple>
  struct explicit_results_check;

  template <typename Result, typename... From>
  struct explicit_results_check<Result, std::tuple<From...>> {
    static constexpr bool
        value = (is_explicit_result_convertible_v<From, Result>
                 && ...);

    static constexpr void assert_each() {
      static_assert_explicit_results<Result, From...>(
          std::index_sequence_for<From...>{});
    }
  };

  template <typename Result>
  inline constexpr bool is_deduced_result_v = std::
      is_same_v<Result, core::dsl::detail::deduced_result>;

  template <typename Result>
  inline constexpr bool is_explicit_result_type_valid_v =
      !std::is_array_v<Result> && !std::is_function_v<Result>
      && !std::is_abstract_v<std::remove_cv_t<Result>>;

  template <typename Result>
  constexpr void static_assert_explicit_result_type() {
    static_assert(
        !std::is_array_v<Result> && !std::is_function_v<Result>,
        "[Patternia.on]: on<R>(...) needs R to be void, an "
        "object type or a reference; a function cannot "
        "return an array or a function type.");
    static_assert(!std::is_abstract_v<std::remove_cv_t<Result>>,
                  "[Patternia.on]: on<R>(...) cannot return an "
                  "abstract class by value. Use a reference or a "
                  "pointer as R.");
  }

  // Whether a borrowed case's handler accepts its pattern's
  // bindings, tested with std::is_invocable on each binding. Unlike
  // is_handler_invocable_v, it never names std::apply, whose MSVC
  // definition is not SFINAE-friendly: a hard error there would
  // replace the case's own diagnostic.
  template <typename Handler, typename Bindings>
  struct is_handler_invocable_with_bindings;

  template <typename Handler, typename... Bindings>
  struct is_handler_invocable_with_bindings<Handler,
                                            std::tuple<Bindings...>>
      : std::bool_constant<std::is_invocable_v<Handler, Bindings...>
                           || std::is_invocable_v<Handler>> {};

  template <typename Case, typename Subject>
  inline constexpr bool is_borrowed_case_invocable_v =
      is_handler_invocable_with_bindings<
          traits::case_handler_t<Case>,
          pat::base::binding_args_t<traits::case_pattern_t<Case>,
                                    Subject>>::value;

  template <typename S, typename Cases, std::size_t... I>
  auto owned_case_results(std::index_sequence<I...>)
      -> std::tuple<core::common::detail::owned_case_result_t<
          decltype(ptn::core::dsl::detail::case_get<I>(
              std::declval<Cases>())),
          S>...>;

  template <typename TV>
  class [[nodiscard(
      "[Patternia.match]: incomplete match expression. "
      "Use `match(subject) | on(...)`.")]] pipeline_match_context {

  public:
    using subject_type = TV;
    static_assert(
        core::common::is_subject_type_valid_v<subject_type>,
        "[Patternia.match]: context must borrow an lvalue or "
        "own a subject object.");

  private:
    subject_type subject_;

    template <typename Result,
              typename... Cases,
              typename CasesStorage>
    PTN_DETAIL_ALWAYS_INLINE constexpr decltype(auto)
    eval_on_cases(CasesStorage &&pipeline_cases) && {
      if constexpr ((traits::is_case_expr_v<Cases> && ...)) {
        return std::move(*this)
            .template eval_valid_on_cases<Result, Cases...>(
                std::forward<CasesStorage>(pipeline_cases));
      }
      else {
        // Stop before handler deduction and dispatch planning can
        // produce unrelated errors for a malformed case pack.
        (ptn::core::common::static_assert_case_expression<Cases>(),
         ...);
        return traits::detail::unreachable_t{};
      }
    }

    template <typename Result,
              typename... Cases,
              typename CasesStorage>
    PTN_DETAIL_ALWAYS_INLINE constexpr decltype(auto)
    eval_valid_on_cases(CasesStorage &&pipeline_cases) && {
      ptn::core::common::
          static_assert_no_unreachable_alt_after_plain_alt<
              Cases...>();

      constexpr bool no_unreachable_cases =
          !ptn::core::common::has_unreachable_case_v<Cases...>;
      static_assert(no_unreachable_cases,
                    "[Patternia.on]: case sequence contains "
                    "unreachable cases. "
                    "Tip: ensure wildcard '_' is last and remove "
                    "shadowed cases.");

      constexpr bool has_pattern_fallback_in_on =
          (traits::is_pattern_fallback_v<
               traits::case_pattern_t<Cases>>
           || ...);
      constexpr bool ends_with_binding_fallback = [] {
        if constexpr (sizeof...(Cases) == 0) {
          return false;
        }
        else {
          using last_case = std::tuple_element_t<
              sizeof...(Cases) - 1,
              std::tuple<Cases...>>;
          return traits::is_unconditional_binding_v<
              traits::case_pattern_t<last_case>>;
        }
      }();
      static_assert(
          has_pattern_fallback_in_on || ends_with_binding_fallback,
          "[Patternia.on]: missing fallback. "
          "Tip: add '_ >> handler' or '$ >> handler' "
          "as the last case (without a guard).");

      if constexpr (!is_deduced_result_v<Result>) {
        return std::move(*this)
            .template eval_explicit_result_cases<Result, Cases...>(
                std::forward<CasesStorage>(pipeline_cases));
      }
      else if constexpr (!std::is_lvalue_reference_v<subject_type>) {
        // owned_case_result_t validates each handler against the
        // actual owned bindings and the case pack's value category.
        using result_info = decltype(core::common::detail::
                                         owned_result_type<
                                             subject_type,
                                             CasesStorage &&>(
                                             std::index_sequence_for<
                                                 Cases...>{}));
        static_assert(
            result_info::valid,
            "[Patternia.match]: handlers must all return void or "
            "values with a common result type.");
        if constexpr (result_info::valid) {
          return core::common::detail::eval_owned_cases<
              typename result_info::type>(
              subject_, std::forward<CasesStorage>(pipeline_cases));
        }
      }
      else {
        (ptn::core::common::static_assert_valid_case<Cases,
                                                     subject_type>(),
         ...);
        auto dummy_fallback = [](auto &&...)
            -> ptn::core::traits::detail::unreachable_t {
          return {};
        };
        using dummy_handler_t = decltype(dummy_fallback);

        ptn::core::common::static_assert_valid_match<subject_type,
                                                     dummy_handler_t,
                                                     Cases...>();

        using result_type = core::traits::
            match_result_t<subject_type, dummy_handler_t, Cases...>;

        if constexpr (traits::is_void_like_v<result_type>) {
          match_impl::eval<result_type>(
              std::forward<subject_type>(subject_),
              std::forward<CasesStorage>(pipeline_cases),
              std::move(dummy_fallback));
        }
        else {
          return match_impl::eval<result_type>(
              std::forward<subject_type>(subject_),
              std::forward<CasesStorage>(pipeline_cases),
              std::move(dummy_fallback));
        }
      }
    }

    // `on<Result>(...)`: the handlers' results need no common type;
    // each one must convert to `Result`.
    template <typename Result,
              typename... Cases,
              typename CasesStorage>
    PTN_DETAIL_ALWAYS_INLINE constexpr decltype(auto)
    eval_explicit_result_cases(CasesStorage &&pipeline_cases) && {
      static_assert_explicit_result_type<Result>();

      if constexpr (!is_explicit_result_type_valid_v<Result>) {
        // Stop before checking handler results against an invalid R.
        return traits::detail::unreachable_t{};
      }
      else if constexpr (!std::is_lvalue_reference_v<subject_type>) {
        // owned_case_result_t validates each handler against the
        // actual owned bindings and the case pack's value category.
        using results_t = decltype(owned_case_results<
                                   subject_type,
                                   CasesStorage &&>(
            std::index_sequence_for<Cases...>{}));
        using check_t   = explicit_results_check<Result, results_t>;
        check_t::assert_each();
        if constexpr (check_t::value) {
          return core::common::detail::eval_owned_cases<Result>(
              subject_, std::forward<CasesStorage>(pipeline_cases));
        }
        else {
          return traits::detail::unreachable_t{};
        }
      }
      else {
        (ptn::core::common::static_assert_valid_case<Cases,
                                                     subject_type>(),
         ...);
        if constexpr ((is_borrowed_case_invocable_v<Cases,
                                                    subject_type>
                       && ...)) {
          using check_t = explicit_results_check<
              Result,
              std::tuple<
                  traits::case_result_t<subject_type, Cases>...>>;
          check_t::assert_each();
          if constexpr (check_t::value) {
            auto dummy_fallback = [](auto &&...)
                -> ptn::core::traits::detail::unreachable_t {
              return {};
            };
            return match_impl::eval<Result>(
                std::forward<subject_type>(subject_),
                std::forward<CasesStorage>(pipeline_cases),
                std::move(dummy_fallback));
          }
          else {
            return traits::detail::unreachable_t{};
          }
        }
        else {
          return traits::detail::unreachable_t{};
        }
      }
    }

  public:
    static constexpr auto create(subject_type &&subject) {
      return pipeline_match_context{
          std::forward<subject_type>(subject)};
    }

    explicit constexpr pipeline_match_context(subject_type &&subject)
        : subject_(std::forward<subject_type>(subject)) {
    }

    template <typename Result, typename... Cases>
    PTN_DETAIL_ALWAYS_INLINE constexpr decltype(auto) operator|(
        core::dsl::detail::on_cases<Result, Cases...> &on_cases) && {
      return std::move(*this)
          .template eval_on_cases<Result, Cases...>(on_cases.cases);
    }

    template <typename Result, typename... Cases>
    PTN_DETAIL_ALWAYS_INLINE constexpr decltype(auto)
    operator|(const core::dsl::detail::on_cases<Result, Cases...>
                  &on_cases) && {
      return std::move(*this)
          .template eval_on_cases<Result, Cases...>(on_cases.cases);
    }

    template <typename Result, typename... Cases>
    PTN_DETAIL_ALWAYS_INLINE constexpr decltype(auto)
    operator|(core::dsl::detail::on_cases<Result, Cases...>
                  &&on_cases) && {
      return std::move(*this)
          .template eval_on_cases<Result, Cases...>(
              std::move(on_cases.cases));
    }
  };

} // namespace ptn::core::engine::detail
