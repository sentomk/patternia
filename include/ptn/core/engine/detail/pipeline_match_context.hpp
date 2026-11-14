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
#include "ptn/core/common/owned_eval.hpp"
#include "ptn/core/dsl/detail/case_expr_impl.hpp"
#include "ptn/core/engine/detail/match_impl.hpp"

namespace ptn::core::dsl::detail {
  template <typename... Cases>
  struct on;
} // namespace ptn::core::dsl::detail

namespace ptn::core::engine::detail {

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

    template <typename... Cases, typename CasesStorage>
    constexpr decltype(auto)
    eval_on_cases(CasesStorage &&pipeline_cases) && {
      if constexpr ((traits::is_case_expr_v<Cases> && ...)) {
        return std::move(*this)
            .template eval_valid_on_cases<Cases...>(
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

    template <typename... Cases, typename CasesStorage>
    constexpr decltype(auto)
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

      if constexpr (!std::is_lvalue_reference_v<subject_type>) {
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

  public:
    static constexpr auto create(subject_type &&subject) {
      return pipeline_match_context{
          std::forward<subject_type>(subject)};
    }

    explicit constexpr pipeline_match_context(subject_type &&subject)
        : subject_(std::forward<subject_type>(subject)) {
    }

    template <typename... Cases>
    constexpr decltype(auto)
    operator|(core::dsl::detail::on<Cases...> &on_cases) && {
      return std::move(*this).template eval_on_cases<Cases...>(
          on_cases.cases);
    }

    template <typename... Cases>
    constexpr decltype(auto)
    operator|(core::dsl::detail::on<Cases...> &&on_cases) && {
      return std::move(*this).template eval_on_cases<Cases...>(
          std::move(on_cases.cases));
    }
  };

} // namespace ptn::core::engine::detail
