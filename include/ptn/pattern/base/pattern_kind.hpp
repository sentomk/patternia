#pragma once

#include <cstdint>
namespace ptn::pat::base {

  enum class pattern_kind : std::uint8_t {
    literal,
    relational,
    predicate,
    type,
    combinator,
    wildcard,
    unknown
  };

} // namespace ptn::pat::base
