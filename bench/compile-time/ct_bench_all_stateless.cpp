// compile-time correctness probe for all_stateless_v. Failure here = perf gate breaker.

#include "ptn/patternia.hpp"

// Named lambdas required: C++17 forbids lambdas in unevaluated operands (decltype/sizeof).
namespace {
constexpr auto h1 = [] { return 1; };
constexpr auto h2 = [] { return 2; };
constexpr auto h0 = [] { return 0; };
} // namespace

// Positive: 3-case empty-handler pack (lambdas are stateless/empty)
static_assert(
    ptn::core::traits::all_stateless_v<
        decltype(ptn::val<1> >> h1),
        decltype(ptn::val<2> >> h2),
        decltype(ptn::_ >> h0)>,
    "empty-handler stateless pack must pass");

// Negative: value_handler<int> is non-empty — must NOT pass
static_assert(
    !ptn::core::traits::all_stateless_v<
        decltype(ptn::val<1> >> 1),
        decltype(ptn::_ >> 0)>,
    "value_handler<int> is non-empty — must NOT pass");

// Size probe: sanity check that on(...) with 3 stateless cases doesn't bloat
static_assert(
    sizeof(decltype(ptn::on(ptn::val<1> >> h1,
                            ptn::val<2> >> h2,
                            ptn::_ >> h0))) <= 256,
    "no unexpected bloat");
