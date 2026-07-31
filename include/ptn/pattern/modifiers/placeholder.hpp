#pragma once

#include <cstddef>

namespace ptn::pat::mod {

  // Internal placeholder node for the Nth bound value.
  template <std::size_t I>
  struct arg_t {
    static constexpr std::size_t index = I;
  };

  // Computes the largest binding position referenced by an
  // expression.
  template <typename E>
  struct max_arg_index;

} // namespace ptn::pat::mod
