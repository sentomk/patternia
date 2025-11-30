#pragma once
namespace ptn::pat::base {

  enum class pattern_kind {
    literal,
    relational,
    predicate,
    type,
    combinator,
    wildcard,
    unknown
  };

} // namespace ptn::pat::base
