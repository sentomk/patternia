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

  int x = 7;
  int result = match(x) | on(any(ProbePattern{false, &c1},
                                 ProbePattern{true, &c2},
                                 ProbePattern{true, &c3})
                                 >> 1,
                             __ >> 0);

  EXPECT_EQ(result, 1);
  EXPECT_EQ(c1, 1);
  EXPECT_EQ(c2, 1);
  EXPECT_EQ(c3, 0);
}

TEST(CombinatorPattern, AnyFallsBackWhenAllMiss) {
  int c1 = 0;
  int c2 = 0;

  int x = 7;
  int result = match(x) | on(any(ProbePattern{false, &c1},
                                 ProbePattern{false, &c2})
                                 >> 1,
                             __ >> 0);

  EXPECT_EQ(result, 0);
  EXPECT_EQ(c1, 1);
  EXPECT_EQ(c2, 1);
}

TEST(CombinatorPattern, AllMatchesOnlyWhenAllHit) {
  int c1 = 0;
  int c2 = 0;
  int c3 = 0;

  int x = 7;
  int result = match(x) | on(all(ProbePattern{true, &c1},
                                 ProbePattern{true, &c2},
                                 ProbePattern{true, &c3})
                                 >> 1,
                             __ >> 0);

  EXPECT_EQ(result, 1);
  EXPECT_EQ(c1, 1);
  EXPECT_EQ(c2, 1);
  EXPECT_EQ(c3, 1);
}

TEST(CombinatorPattern, AllShortCircuitsOnFirstMiss) {
  int c1 = 0;
  int c2 = 0;
  int c3 = 0;

  int x = 7;
  int result = match(x) | on(all(ProbePattern{true, &c1},
                                 ProbePattern{false, &c2},
                                 ProbePattern{true, &c3})
                                 >> 1,
                             __ >> 0);

  EXPECT_EQ(result, 0);
  EXPECT_EQ(c1, 1);
  EXPECT_EQ(c2, 1);
  EXPECT_EQ(c3, 0);
}

TEST(CombinatorPattern, AnyAndAllWorkWithZeroBindHandlers) {
  int x = 2;

  int any_result = match(x) | on(any(lit(1), lit(2)) >> [] { return 11; },
                                 __ >> 0);

  int all_result = match(x) | on(all(any(lit(2), lit(3)), lit(2))
                                     >> [] { return 22; },
                                 __ >> 0);

  EXPECT_EQ(any_result, 11);
  EXPECT_EQ(all_result, 22);
}

TEST(CombinatorPattern, AnyWithValStaticLiterals) {
  int x = 2;
  int result = match(x) | on(any(val<1>, val<2>, val<3>) >> 1, __ >> 0);

  EXPECT_EQ(result, 1);
}

TEST(CombinatorPattern, AnyWithValStaticLiteralsMiss) {
  int x = 5;
  int result = match(x) | on(any(val<1>, val<2>, val<3>) >> 1, __ >> 0);

  EXPECT_EQ(result, 0);
}

TEST(CombinatorPattern, AllWithValStaticLiterals) {
  int x = 2;
  int result = match(x) | on(all(val<2>, val<2>) >> 1, __ >> 0);

  EXPECT_EQ(result, 1);
}

TEST(CombinatorPattern, AllWithValStaticLiteralsMiss) {
  int x = 2;
  int result = match(x) | on(all(val<1>, val<2>) >> 1, __ >> 0);

  EXPECT_EQ(result, 0);
}

TEST(CombinatorPattern, AnyWithIsTypePatterns) {
  std::variant<int, std::string, double> v = std::string("hello");

  int result = match(v)
               | on(any(is<int>, is<double>) >> 1,
                    is<std::string> >> 2,
                    __ >> 0);

  EXPECT_EQ(result, 2);
}

TEST(CombinatorPattern, AnyMatchesVariantType) {
  std::variant<int, std::string, double> v = 42;

  int result = match(v)
               | on(any(is<int>, is<double>) >> 1,
                    is<std::string> >> 2,
                    __ >> 0);

  EXPECT_EQ(result, 1);
}

TEST(CombinatorPattern, SingleSubPatternAny) {
  int x = 7;
  int result = match(x) | on(any(lit(7)) >> 1, __ >> 0);

  EXPECT_EQ(result, 1);
}

TEST(CombinatorPattern, SingleSubPatternAll) {
  int x = 7;
  int result = match(x) | on(all(lit(7)) >> 1, __ >> 0);

  EXPECT_EQ(result, 1);
}

TEST(CombinatorPattern, NestedAnyInsideAll) {
  int x = 4;
  int result = match(x)
               | on(all(any(lit(1), lit(2), lit(3), lit(4)), val<4>) >> 42,
                   __ >> 0);

  EXPECT_EQ(result, 42);
}

TEST(CombinatorPattern, NestedAllInsideAny) {
  int x = 5;
  int result = match(x)
               | on(any(all(val<5>, val<5>), all(val<6>, val<6>)) >> 99,
                   __ >> 0);

  EXPECT_EQ(result, 99);
}

TEST(CombinatorPattern, NestedAnyMissOuterAllHit) {
  int x = 10;
  int result = match(x)
               | on(all(any(lit(1), lit(2)), val<10>) >> 1,
                   __ >> 0);

  EXPECT_EQ(result, 0);
}

TEST(CombinatorPattern, AnyWithLitCi) {
  std::string s = "HeLLo";
  int result = match(s)
               | on(any(lit_ci("hello"), lit_ci("world")) >> 1, __ >> 0);

  EXPECT_EQ(result, 1);
}

TEST(CombinatorPattern, AnyReusedAcrossMatches) {
  auto pattern = any(val<1>, val<2>, val<3>);

  int a = 1;
  int b = 2;
  int c = 4;

  EXPECT_EQ(match(a) | on(pattern >> 1, __ >> 0), 1);
  EXPECT_EQ(match(b) | on(pattern >> 1, __ >> 0), 1);
  EXPECT_EQ(match(c) | on(pattern >> 1, __ >> 0), 0);
}

TEST(CombinatorPattern, AllReusedAcrossMatches) {
  auto pattern = all(any(val<1>, val<2>), val<2>);

  int a = 2;
  int b = 3;

  EXPECT_EQ(match(a) | on(pattern >> 1, __ >> 0), 1);
  EXPECT_EQ(match(b) | on(pattern >> 1, __ >> 0), 0);
}

TEST(CombinatorPattern, AnyInsidePtnOnMacro) {
  int a = 1;
  int b = 3;
  int c = 5;

  auto run = [](int x) {
    return match(x) | PTN_ON(
        any(val<1>, val<2>, val<3>) >> 1,
        __ >> 0);
  };

  EXPECT_EQ(run(a), 1);
  EXPECT_EQ(run(b), 1);
  EXPECT_EQ(run(c), 0);
}

TEST(CombinatorPattern, AllInsidePtnOnMacro) {
  int a = 2;
  int b = 3;

  auto run = [](int x) {
    return match(x) | PTN_ON(
        all(any(val<1>, val<2>), val<2>) >> 1,
        __ >> 0);
  };

  EXPECT_EQ(run(a), 1);
  EXPECT_EQ(run(b), 0);
}
