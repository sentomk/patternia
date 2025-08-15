#ifndef PATTERNIA_HPP
#define PATTERNIA_HPP

#include <tuple>
#include <type_traits>
#include <utility>
#include "ptn/core/match_builder.hpp"
#include "ptn/config.hpp"

namespace ptn {
  /* wrapper: the only public entry to start a match chain */
  template <typename T>
  constexpr auto match(T &&value) noexcept(std::is_nothrow_constructible_v<std::decay_t<T>, T &&>) {
    using V = std::decay_t<T>;
    return core::match_builder<V>(V(std::forward<T>(value)), std::tuple<>{}, core::ctor_tag{});
  }
}; // namespace ptn

#if PTN_ENABLE_VALUE_PATTERN
// clang-format off
#  include <ptn/patterns/value.hpp>
// clang-format on
#endif

#if PTN_ENABLE_RELATIONAL_PATTERN
// clang-format off
#  include <ptn/patterns/relational.hpp>
// clang-format on
#endif

#endif // PATTERNIA_HPP