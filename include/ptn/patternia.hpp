#ifndef PATTERNIA_HPP
#define PATTERNIA_HPP

#include <tuple>
#include <type_traits>
#include <utility>
#include "ptn/core/match_builder.hpp" // core API
#include "ptn/config.hpp"

namespace ptn {

  using namespace core;
  using namespace patterns;
  using namespace dsl;

  /* wrapper: the only public entry to start a match chain */
  template <typename T>
  constexpr auto match(T &&value) noexcept(
      std::is_nothrow_constructible_v<std::decay_t<T>, T &&>) {
    using V = std::decay_t<T>;
    return core::match_builder<V>::create(
        V(std::forward<T>(value)), std::tuple<>{});
  }
}; // namespace ptn

#if PTN_ENABLE_VALUE_PATTERN
// clang-format off
#  include <ptn/patterns/value.hpp>
// clang-format on
#endif

#if PTN_ENABLE_RELATIONAL_PATTERN
// clang-format off
#include <ptn/patterns/relational.hpp>
// clang-format on
#endif

#endif // PATTERNIA_HPP