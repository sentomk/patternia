#include <gtest/gtest.h>

#include <ptn/patternia.hpp>

#include <string>
#include <tuple>
#include <variant>

using namespace ptn;

struct ProbePattern : ptn::pat::base::pattern_base<ProbePattern> {
  bool should_match;
  int *calls;

  constexpr ProbePattern(bool should_match, int *calls) noexcept
      : should_match(should_match), calls(calls) {
  }

  template <typename Subject>
  constexpr bool match(const Subject &) const noexcept {
    ++(*calls);
    return should_match;
  }

  template <typename Subject>
  constexpr auto bind(const Subject &) const {
    return std::tuple<>{};
  }
};

namespace ptn::pat::base {

  template <typename Subject>
  struct binding_args<::ProbePattern, Subject> {
    using type = std::tuple<>;
  };

} // namespace ptn::pat::base

TEST(CombinatorPattern, AnyShortCircuitsAfterFirstHit) {
  int c1 = 0;
  int c2 = 0;
  int c3 = 0;

  int x      = 7;
  int result = match(x)
               | on(any(ProbePattern{false, &c1},
                        ProbePattern{true, &c2},
                        ProbePattern{true, &c3})
                        >> 1,
                    _ >> 0);

  EXPECT_EQ(result, 1);
  EXPECT_EQ(c1, 1);
  EXPECT_EQ(c2, 1);
  EXPECT_EQ(c3, 0);
}

TEST(CombinatorPattern, AnyFallsBackWhenAllMiss) {
  int c1 = 0;
  int c2 = 0;

  int x      = 7;
  int result = match(x)
               | on(any(ProbePattern{false, &c1},
                        ProbePattern{false, &c2})
                        >> 1,
                    _ >> 0);

  EXPECT_EQ(result, 0);
  EXPECT_EQ(c1, 1);
  EXPECT_EQ(c2, 1);
}

TEST(CombinatorPattern, AllMatchesOnlyWhenAllHit) {
  int c1 = 0;
  int c2 = 0;
  int c3 = 0;

  int x      = 7;
  int result = match(x)
               | on(all(ProbePattern{true, &c1},
                        ProbePattern{true, &c2},
                        ProbePattern{true, &c3})
                        >> 1,
                    _ >> 0);

  EXPECT_EQ(result, 1);
  EXPECT_EQ(c1, 1);
  EXPECT_EQ(c2, 1);
  EXPECT_EQ(c3, 1);
}

TEST(CombinatorPattern, AllShortCircuitsOnFirstMiss) {
  int c1 = 0;
  int c2 = 0;
  int c3 = 0;

  int x      = 7;
  int result = match(x)
               | on(all(ProbePattern{true, &c1},
                        ProbePattern{false, &c2},
                        ProbePattern{true, &c3})
                        >> 1,
                    _ >> 0);

  EXPECT_EQ(result, 0);
  EXPECT_EQ(c1, 1);
  EXPECT_EQ(c2, 1);
  EXPECT_EQ(c3, 0);
}

TEST(CombinatorPattern, AnyAndAllWorkWithZeroBindHandlers) {
  int x = 2;

  int any_result = match(x)
                   | on(
                       any(lit(1), lit(2)) >> [] { return 11; },
                       _ >> 0);

  int all_result = match(x)
                   | on(
                       all(any(lit(2), lit(3)), lit(2)) >>
                           [] { return 22; },
                       _ >> 0);

  EXPECT_EQ(any_result, 11);
  EXPECT_EQ(all_result, 22);
}

TEST(CombinatorPattern, AnyWithValStaticLiterals) {
  int x      = 2;
  int result = match(x)
               | on(any(val<1>, val<2>, val<3>) >> 1, _ >> 0);

  EXPECT_EQ(result, 1);
}

TEST(CombinatorPattern, AnyWithValStaticLiteralsMiss) {
  int x      = 5;
  int result = match(x)
               | on(any(val<1>, val<2>, val<3>) >> 1, _ >> 0);

  EXPECT_EQ(result, 0);
}

TEST(CombinatorPattern, AllWithValStaticLiterals) {
  int x      = 2;
  int result = match(x) | on(all(val<2>, val<2>) >> 1, _ >> 0);

  EXPECT_EQ(result, 1);
}

TEST(CombinatorPattern, AllWithValStaticLiteralsMiss) {
  int x      = 2;
  int result = match(x) | on(all(val<1>, val<2>) >> 1, _ >> 0);

  EXPECT_EQ(result, 0);
}

TEST(CombinatorPattern, AnyWithIsTypePatterns) {
  std::variant<int, std::string, double> v = std::string("hello");

  int result = match(v)
               | on(any(is<int>, is<double>) >> 1,
                    is<std::string> >> 2,
                    _ >> 0);

  EXPECT_EQ(result, 2);
}

TEST(CombinatorPattern, AnyMatchesVariantType) {
  std::variant<int, std::string, double> v = 42;

  int result = match(v)
               | on(any(is<int>, is<double>) >> 1,
                    is<std::string> >> 2,
                    _ >> 0);

  EXPECT_EQ(result, 1);
}

TEST(CombinatorPattern, SingleSubPatternAny) {
  int x      = 7;
  int result = match(x) | on(any(lit(7)) >> 1, _ >> 0);

  EXPECT_EQ(result, 1);
}

TEST(CombinatorPattern, SingleSubPatternAll) {
  int x      = 7;
  int result = match(x) | on(all(lit(7)) >> 1, _ >> 0);

  EXPECT_EQ(result, 1);
}

TEST(CombinatorPattern, NestedAnyInsideAll) {
  int x      = 4;
  int result = match(x)
               | on(all(any(lit(1), lit(2), lit(3), lit(4)), val<4>)
                        >> 42,
                    _ >> 0);

  EXPECT_EQ(result, 42);
}

TEST(CombinatorPattern, NestedAllInsideAny) {
  int x      = 5;
  int result = match(x)
               | on(any(all(val<5>, val<5>), all(val<6>, val<6>))
                        >> 99,
                    _ >> 0);

  EXPECT_EQ(result, 99);
}

TEST(CombinatorPattern, NestedAnyMissOuterAllHit) {
  int x      = 10;
  int result = match(x)
               | on(all(any(lit(1), lit(2)), val<10>) >> 1, _ >> 0);

  EXPECT_EQ(result, 0);
}

TEST(CombinatorPattern, AnyWithLitCi) {
  std::string s      = "HeLLo";
  int         result = match(s)
               | on(any(lit_ci("hello"), lit_ci("world")) >> 1,
                    _ >> 0);

  EXPECT_EQ(result, 1);
}

TEST(CombinatorPattern, AnyReusedAcrossMatches) {
  auto pattern = any(val<1>, val<2>, val<3>);

  int a = 1;
  int b = 2;
  int c = 4;

  EXPECT_EQ(match(a) | on(pattern >> 1, _ >> 0), 1);
  EXPECT_EQ(match(b) | on(pattern >> 1, _ >> 0), 1);
  EXPECT_EQ(match(c) | on(pattern >> 1, _ >> 0), 0);
}

TEST(CombinatorPattern, AllReusedAcrossMatches) {
  auto pattern = all(any(val<1>, val<2>), val<2>);

  int a = 2;
  int b = 3;

  EXPECT_EQ(match(a) | on(pattern >> 1, _ >> 0), 1);
  EXPECT_EQ(match(b) | on(pattern >> 1, _ >> 0), 0);
}

TEST(CombinatorPattern, AnyInsidePtnOnMacro) {
  int a = 1;
  int b = 3;
  int c = 5;

  auto run = [](int x) {
    return match(x)
           | PTN_ON(any(val<1>, val<2>, val<3>) >> 1, _ >> 0);
  };

  EXPECT_EQ(run(a), 1);
  EXPECT_EQ(run(b), 1);
  EXPECT_EQ(run(c), 0);
}

TEST(CombinatorPattern, AllInsidePtnOnMacro) {
  int a = 2;
  int b = 3;

  auto run = [](int x) {
    return match(x)
           | PTN_ON(all(any(val<1>, val<2>), val<2>) >> 1, _ >> 0);
  };

  EXPECT_EQ(run(a), 1);
  EXPECT_EQ(run(b), 0);
}

// ===== neg(p) negation pattern =====

TEST(NegationPattern, BasicNegation) {
  int a = 1, b = 2;
  EXPECT_EQ(match(a)
                | on(
                    neg(val<1>) >> [] { return 0; },
                    _ >> [] { return 1; }),
            1);
  EXPECT_EQ(match(b)
                | on(
                    neg(val<1>) >> [] { return 0; },
                    _ >> [] { return 1; }),
            0);
}

TEST(NegationPattern, NegWildcardNeverMatches) {
  int x = 42;
  // neg(_) matches nothing — no subject fails the wildcard
  EXPECT_EQ(
      match(x)
          | on(
              neg(_) >> [] { return 0; }, _ >> [] { return 1; }),
      1);
}

TEST(NegationPattern, NegWithPred) {
  int  a = 3, b = 4;
  auto is_even = [](int x) { return x % 2 == 0; };
  EXPECT_EQ(match(a)
                | on(
                    neg(pred(is_even)) >> [] { return 0; },
                    _ >> [] { return 1; }),
            0);
  EXPECT_EQ(match(b)
                | on(
                    neg(pred(is_even)) >> [] { return 0; },
                    _ >> [] { return 1; }),
            1);
}

TEST(NegationPattern, NegNegIsIdentity) {
  int a = 1, b = 2;
  // double negation: neg(neg(p)) matches iff p matches
  EXPECT_EQ(match(a)
                | on(
                    neg(neg(val<1>)) >> [] { return 42; },
                    _ >> [] { return 0; }),
            42);
  EXPECT_EQ(match(b)
                | on(
                    neg(neg(val<2>)) >> [] { return 42; },
                    _ >> [] { return 0; }),
            42);
}

// =========================================================================
// Operator sugar: !p == neg(p), (a || b) == any(a, b),
// (a && b) == all(a, b).
// =========================================================================

TEST(OperatorSugar, NegationIsNeg) {
  static_assert(
      std::is_same_v<decltype(!lit(1)), decltype(neg(lit(1)))>);
}

TEST(OperatorSugar, BangMatchesNeg) {
  int a = 5, b = 1;
  EXPECT_EQ(match(a) | on(!lit(1) >> 1, _ >> 0), 1);
  EXPECT_EQ(match(b) | on(!lit(1) >> 1, _ >> 0), 0);
}

TEST(OperatorSugar, BangOnVal) {
  int a = 404, b = 200;
  EXPECT_EQ(match(a) | on(!val<200> >> 1, _ >> 0), 1);
  EXPECT_EQ(match(b) | on(!val<200> >> 1, _ >> 0), 0);
}

TEST(OperatorSugar, OrIsAny) {
  int a = 2, b = 3;
  // NOTE: `>>` binds tighter than `||`, hence the parentheses.
  EXPECT_EQ(match(a) | on((lit(1) || lit(2)) >> 1, _ >> 0), 1);
  EXPECT_EQ(match(b) | on((lit(1) || lit(2)) >> 1, _ >> 0), 0);
}

TEST(OperatorSugar, OrChainsLeft) {
  int a = 3, b = 4;
  EXPECT_EQ(match(a) | on((lit(1) || lit(2) || lit(3)) >> 1, _ >> 0),
            1);
  EXPECT_EQ(match(b) | on((lit(1) || lit(2) || lit(3)) >> 1, _ >> 0),
            0);
}

TEST(OperatorSugar, AndIsAll) {
  auto is_even = [](int x) { return x % 2 == 0; };
  int  a = 6, b = 2, c = 5;
  EXPECT_EQ(match(a) | on((pred(is_even) && !lit(2)) >> 1, _ >> 0),
            1);
  EXPECT_EQ(match(b) | on((pred(is_even) && !lit(2)) >> 1, _ >> 0),
            0);
  EXPECT_EQ(match(c) | on((pred(is_even) && !lit(2)) >> 1, _ >> 0),
            0);
}

TEST(OperatorSugar, DoubleBangIsIdentity) {
  int a = 1, b = 2;
  EXPECT_EQ(match(a) | on(!!lit(1) >> 1, _ >> 0), 1);
  EXPECT_EQ(match(b) | on(!!lit(1) >> 1, _ >> 0), 0);
}

TEST(OperatorSugar, GuardOperatorsUnaffected) {
  // `&&` / `||` between guard predicates keep pred_and / pred_or
  // semantics; the pattern-level sugar must not interfere.
  int a = 50, b = 150;
  EXPECT_EQ(match(a) | on($[(_ > 0) && (_ < 100)] >> 1, _ >> 0), 1);
  EXPECT_EQ(match(b) | on($[(_ < 0) || (_ > 100)] >> 1, _ >> 0), 1);
}

TEST(OperatorSugar, MixedWithTypePatterns) {
  // The sugar composes patterns of different kinds against one
  // subject (here: two type patterns over a variant).
  std::variant<int, std::string, double> a = 1;
  std::variant<int, std::string, double> b = std::string("s");
  std::variant<int, std::string, double> c = 1.5;
  EXPECT_EQ(
      match(a)
          | on((is<int> || is<std::string>) >> [] { return 1; },
               _ >> 0),
      1);
  EXPECT_EQ(
      match(b)
          | on((is<int> || is<std::string>) >> [] { return 1; },
               _ >> 0),
      1);
  EXPECT_EQ(
      match(c)
          | on((is<int> || is<std::string>) >> [] { return 1; },
               _ >> 0),
      0);
}

// --- Operator sugar truth table: exercise every !/||/&& combination
// over a small predicate set so each operator branch is hit. ---

TEST(OperatorSugar, FullTruthTable) {
  auto pos  = [](int n) { return n > 0; };
  auto even = [](int n) { return n % 2 == 0; };

  auto run = [&](int v, auto pat) {
    return ptn::match(v) | ptn::on(pat >> 1, ptn::_ >> 0);
  };

  // (a || b) : both true, one true, none true
  EXPECT_EQ(run(4, ptn::pred(pos) || ptn::pred(even)), 1);
  EXPECT_EQ(run(-2, ptn::pred(pos) || ptn::pred(even)), 1);
  EXPECT_EQ(run(-3, ptn::pred(pos) || ptn::pred(even)), 0);

  // (a && b) : both true, one false, none true
  EXPECT_EQ(run(4, ptn::pred(pos) && ptn::pred(even)), 1);
  EXPECT_EQ(run(3, ptn::pred(pos) && ptn::pred(even)), 0);
  EXPECT_EQ(run(-2, ptn::pred(pos) && ptn::pred(even)), 0);

  // !p combined with && / ||
  EXPECT_EQ(run(-1, !ptn::pred(pos) && ptn::pred(even)), 0);
  EXPECT_EQ(run(-4, !ptn::pred(pos) && ptn::pred(even)), 1);
  EXPECT_EQ(run(0, !ptn::pred(pos) || ptn::pred(pos)), 1);
}
