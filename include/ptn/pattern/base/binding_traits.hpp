#pragma once
#include <tuple>

namespace ptn::pat::base {

  template <typename Pattern, typename Subject>
  struct binding_args {
    using type = std::tuple<Subject>; // default
  };

  template <typename Pattern, typename Subject>
  using binding_args_t = typename binding_args<Pattern, Subject>::type;

} // namespace ptn::pat::base
