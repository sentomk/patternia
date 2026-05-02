#pragma once
// Shared empty-handler structs for multi-TU ODR-merge verification.
// Both test_on_auto_cache_multi_tu.cpp and test_on_auto_cache_multi_tu_helper.cpp
// include this header and use these types in their match expressions.
// This guarantees literal type-identity for Cases... across TUs, which is
// a prerequisite for ODR merging of the function-local static cache slot.
// NOTE: inline lambdas would produce unique closure types per TU and break this.

namespace auto_cache_shared {
  struct DoubleIt   { int operator()(int x) const { return x * 2; } };
  struct ReturnZero { int operator()() const { return 0; } };
}
