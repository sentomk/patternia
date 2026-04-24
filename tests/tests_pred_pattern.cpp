#include <gtest/gtest.h>

#include <ptn/patternia.hpp>

#include <string>

using namespace ptn;

TEST(PredPattern, MatchesWhenPredicateReturnsTrue) {
  int x      = 4;
  int result = match(x)
               | on(pred([](int v) { return v % 2 == 0; }) >> 1,
                    __ >> 0);

  EXPECT_EQ(result, 1);
}

TEST(PredPattern, FallsBackWhenPredicateReturnsFalse) {
  int x      = 3;
  int result = match(x)
               | on(pred([](int v) { return v % 2 == 0; }) >> 1,
                    __ >> 0);

  EXPECT_EQ(result, 0);
}

TEST(PredPattern, WorksWithStringSubject) {
  std::string s      = "hello";
  int         result = match(s)
               | on(pred([](const std::string &v) {
                      return v.size() > 3;
                    }) >> 1,
                    __ >> 0);

  EXPECT_EQ(result, 1);
}

TEST(PredPattern, FallsBackWithShortString) {
  std::string s      = "hi";
  int         result = match(s)
               | on(pred([](const std::string &v) {
                      return v.size() > 3;
                    }) >> 1,
                    __ >> 0);

  EXPECT_EQ(result, 0);
}

TEST(PredPattern, ComposesWithAny) {
  int x      = 7;
  int result = match(x)
               | on(any(pred([](int v) { return v < 0; }),
                        pred([](int v) { return v > 5; }))
                        >> 1,
                    __ >> 0);

  EXPECT_EQ(result, 1);
}

TEST(PredPattern, ComposesWithAll) {
  int x      = 6;
  int result = match(x)
               | on(all(pred([](int v) { return v > 0; }),
                        pred([](int v) { return v % 2 == 0; }))
                        >> 1,
                    __ >> 0);

  EXPECT_EQ(result, 1);
}

TEST(PredPattern, AllMissWhenOnePredicateFails) {
  int x      = 5;
  int result = match(x)
               | on(all(pred([](int v) { return v > 0; }),
                        pred([](int v) { return v % 2 == 0; }))
                        >> 1,
                    __ >> 0);

  EXPECT_EQ(result, 0);
}

TEST(PredPattern, ReusableAcrossMatches) {
  auto is_positive = pred([](int v) { return v > 0; });

  int pos  = 1;
  int zero = 0;
  int neg  = -1;

  EXPECT_EQ(match(pos) | on(is_positive >> 1, __ >> 0), 1);
  EXPECT_EQ(match(zero) | on(is_positive >> 1, __ >> 0), 0);
  EXPECT_EQ(match(neg) | on(is_positive >> 1, __ >> 0), 0);
}

TEST(PredPattern, WorksInsidePtnOnMacro) {
  auto run = [](int x) {
    return match(x)
           | PTN_ON(pred([](int v) { return v % 2 == 0; }) >> 1,
                    __ >> 0);
  };

  EXPECT_EQ(run(2), 1);
  EXPECT_EQ(run(3), 0);
}

TEST(PredPattern, FullyQualifiedWithoutUsingDirective) {
  int x = 10;

  int result = ptn::match(x)
               | ptn::on(
                   ptn::pred([](int v) { return v == 10; }) >> 42,
                   ptn::__ >> 0);

  EXPECT_EQ(result, 42);
}
