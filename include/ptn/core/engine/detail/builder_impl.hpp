#pragma once
/**
 * @file builder_impl.hpp
 * @brief Implementation of the Core match_builder.
 */

#include <tuple>
#include <type_traits>
#include <utility>

#include "ptn/config.hpp"
#include "ptn/core/common/diagnostics.hpp"
#include "ptn/core/engine/detail/match_impl.hpp"

namespace ptn::core::engine::detail {

  template <typename TV, typename... Cases>
  class match_builder {
  public:
    using subject_type = TV;
    using cases_type   = std::tuple<Cases...>;

  private:
    subject_type subject_;
    cases_type   cases_;

  public:
    /** @brief Static factory with an empty case list. */
    static constexpr auto create(subject_type subject) {
      return match_builder{std::move(subject), cases_type{}};
    }

    /** @brief Static factory with an explicit case tuple (internal use). */
    static constexpr auto create(subject_type subject, cases_type cases) {
      return match_builder{std::move(subject), std::move(cases)};
    }

    /** @brief Construct from subject and case tuple. */
    constexpr match_builder(subject_type subject, cases_type cases)
        : subject_(std::move(subject)), cases_(std::move(cases)) {
    }

    // ----------------- chaining API: .when(...) -----------------

    /**
     * @brief Add a new case (rvalue-qualified).
     *
     * Accepts any CaseExpr-like object; typically `dsl::case_expr<Pattern,
     * Handler>`.
     */
    template <typename CaseExpr>
    constexpr auto when(CaseExpr &&expr) && {
      using new_case_type = std::decay_t<CaseExpr>;

      // Compile-time validation of the new case.
      static_assert(
          ptn::core::common::is_case_expr_v<new_case_type>,
          "Argument to .when() must be a case expression created with the '>>' "
          "operator.");
      static_assert(
          ptn::core::common::
              is_handler_invocable_v<new_case_type, subject_type>,
#if PTN_USE_CONCEPTS
          "Handler cannot be invoked with the arguments bound by the pattern."
#else
          "Handler signature does not match the pattern's binding result."
#endif
      );

      auto new_cases = std::tuple_cat(
          std::move(cases_),
          std::tuple<new_case_type>(std::forward<CaseExpr>(expr)));

      using builder_t = match_builder<subject_type, Cases..., new_case_type>;

      return builder_t{std::move(subject_), std::move(new_cases)};
    }

    /**
     * @brief Add a new case (lvalue-qualified).
     */

    template <typename CaseExpr>
    constexpr auto when(CaseExpr &&expr) const & {
      using new_case_type = std::decay_t<CaseExpr>;

      static_assert(
          ptn::core::common::is_case_expr_v<new_case_type>,
          "Argument to .when() must be a case expression created with the '>>' "
          "operator.");
      static_assert(
          ptn::core::common::
              is_handler_invocable_v<new_case_type, subject_type>,
#if PTN_USE_CONCEPTS
          "Handler cannot be invoked with the arguments bound by the pattern."
#else
          "Handler signature does not match the pattern's binding result."
#endif
      );

      auto new_cases = std::tuple_cat(
          cases_, std::tuple<new_case_type>(std::forward<CaseExpr>(expr)));

      using builder_t = match_builder<subject_type, Cases..., new_case_type>;

      return builder_t{subject_, std::move(new_cases)};
    }

    // ----------------- terminal API: .otherwise(...) ------------

    /**
     * @brief Terminal step: evaluate all cases; if none matches,
     *        call the provided fallback handler.
     */
    template <typename Otherwise>
    constexpr decltype(auto) otherwise(Otherwise &&otherwise_handler) && {

      ptn::core::common::
          static_assert_valid_match<subject_type, Otherwise, Cases...>();

      return match_impl::eval(
          subject_, cases_, std::forward<Otherwise>(otherwise_handler));
    }
  };

} // namespace ptn::core::engine::detail
