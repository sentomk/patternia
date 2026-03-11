#pragma once

// Implementation of the pipeline-only match context.
//
// This header contains the internal object returned by `match(subject)`.
// It exists solely to support immediate evaluation through
// `match(subject) | on(...)`.

#include <tuple>
#include <type_traits>
#include <utility>

#include "ptn/core/common/common_traits.hpp"
#include "ptn/core/common/diagnostics.hpp"
#include "ptn/core/dsl/detail/case_expr_impl.hpp"
#include "ptn/core/engine/detail/match_impl.hpp"

namespace ptn::core::dsl::detail {
  template <typename... Cases>
  struct on;
} // namespace ptn::core::dsl::detail

namespace ptn::core::engine::detail {

  template <typename TV>
  class [[nodiscard("[Patternia.match]: incomplete match expression. "
                    "Use `match(subject) | on(...)`.")]]
      pipeline_match_context {

  public:
    using subject_type = TV;

    using subject_type_check =
        ptn::core::common::subject_type_validator<subject_type>;

  private:
    subject_type subject_;

    template <typename... Cases, typename CasesStorage>
    constexpr decltype(auto)
    eval_on_cases(CasesStorage &&pipeline_cases) && {
      (ptn::core::common::static_assert_valid_case<Cases, subject_type>(), ...);

      ptn::core::common::static_assert_no_unreachable_alt_after_plain_alt<
          Cases...>();

      constexpr bool no_unreachable_cases =
          !ptn::core::common::has_unreachable_case_v<Cases...>;
      static_assert(
          no_unreachable_cases,
          "[Patternia.on]: case sequence contains unreachable cases. "
          "Tip: ensure wildcard '__' is last and remove shadowed cases.");

      constexpr bool has_pattern_fallback_in_on =
          (traits::is_pattern_fallback_v<traits::case_pattern_t<Cases>> || ...);
      static_assert(
          has_pattern_fallback_in_on,
          "[Patternia.on]: missing wildcard '__' fallback. "
          "Tip: add '__ >> handler' as the last case.");

      auto dummy_fallback =
          [](auto &&...) -> ptn::core::traits::detail::unreachable_t {
        return {};
      };
      using dummy_handler_t = decltype(dummy_fallback);

      ptn::core::common::static_assert_valid_match<
          subject_type,
          dummy_handler_t,
          Cases...>();

      using result_type =
          core::traits::match_result_t<subject_type, dummy_handler_t, Cases...>;

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

  public:
    static constexpr auto create(subject_type subject) {
      return pipeline_match_context{std::forward<subject_type>(subject)};
    }

    explicit constexpr pipeline_match_context(subject_type subject)
        : subject_(std::forward<subject_type>(subject)) {
    }

    template <typename... Cases>
    constexpr decltype(auto)
    operator|(core::dsl::detail::on<Cases...> &on_cases) && {
      return std::move(*this).template eval_on_cases<Cases...>(on_cases.cases);
    }

    template <typename... Cases>
    constexpr decltype(auto)
    operator|(core::dsl::detail::on<Cases...> &&on_cases) && {
      return std::move(*this).template eval_on_cases<Cases...>(
          std::move(on_cases.cases));
    }
  };

} // namespace ptn::core::engine::detail
