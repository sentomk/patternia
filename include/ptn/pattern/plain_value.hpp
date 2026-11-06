#pragma once

// Plain values in pattern positions.
//
// Wherever a pattern is expected as an argument (any, all, neg,
// is<T>(...), alt<I>(...) and the right operand of || / &&), a plain
// value is accepted and treated as lit(value):
//
//   any('a', 'e', 'i')    // any(lit('a'), lit('e'), lit('i'))
//   is<int>(0)            // is<int>(lit(0))
//   (lit(' ') || '\n')    // any(lit(' '), lit('\n'))

#include <string_view>
#include <type_traits>
#include <utility>

#include "ptn/pattern/base/pattern_base.hpp"
#include "ptn/pattern/lit.hpp"

namespace ptn::pat::base {

  // A plain value: arithmetic, enumeration and string-like values.
  // Patterns and callables are excluded.
  template <typename T>
  inline constexpr bool
      is_plain_value_v = !std::is_base_of_v<pattern_tag,
                                            std::decay_t<T>>
                         && (std::is_arithmetic_v<std::decay_t<T>>
                             || std::is_enum_v<std::decay_t<T>>
                             || std::is_convertible_v<
                                 std::decay_t<T>,
                                 std::string_view>);

  // Returns lit(x) for a plain value and forwards a pattern
  // unchanged.
  template <typename T>
  constexpr decltype(auto) as_pattern(T &&x) {
    if constexpr (is_plain_value_v<T>) {
      return pat::lit(std::forward<T>(x));
    }
    else {
      return std::forward<T>(x);
    }
  }

  // The pattern type that as_pattern produces for an argument of
  // type T.
  template <typename T>
  using as_pattern_t = std::decay_t<decltype(as_pattern(
      std::declval<T>()))>;

} // namespace ptn::pat::base
