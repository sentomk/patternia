#pragma once

namespace ptn::core::common {

  namespace detail {
    template <typename Subject, typename CasesTuple>
    struct optimize_case_sequence {
      using type = CasesTuple;
    };
  } // namespace detail

  template <typename Subject, typename CasesTuple>
  using optimize_case_sequence_t =
      typename detail::optimize_case_sequence<Subject, CasesTuple>::type;

} // namespace ptn::core::common
