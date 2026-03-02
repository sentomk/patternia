#include <gtest/gtest.h>

#include <ptn/patternia.hpp>

#include <string>

using namespace ptn;

TEST(LiteralPattern, LitMatchesExpectedValue) {
  int x      = 5;
  int result = ptn::match(x).when(ptn::lit(5) >> 42).otherwise(-1);

  EXPECT_EQ(result, 42);
}

TEST(LiteralPattern, LitOtherwiseFallback) {
  int x      = 3;
  int result = ptn::match(x).when(ptn::lit(5) >> 42).otherwise(-1);

  EXPECT_EQ(result, -1);
}

TEST(LiteralPattern, LitVMatchesExpectedValue) {
  int x      = 5;
  int result = ptn::match(x).when(ptn::lit_v<5>() >> 42).otherwise(-1);

  EXPECT_EQ(result, 42);
}

TEST(LiteralPattern, LitVOtherwiseFallback) {
  int x      = 3;
  int result = ptn::match(x).when(ptn::lit_v<5>() >> 42).otherwise(-1);

  EXPECT_EQ(result, -1);
}

TEST(LiteralPattern, LitVSelectsSparseDenseCases) {
  auto cases = on{
      ptn::lit_v<1>() >> 1,
      ptn::lit_v<2>() >> 2,
      ptn::lit_v<10>() >> 10,
      __ >> 0,
  };

  int one   = 1;
  int two   = 2;
  int ten   = 10;
  int other = 7;

  EXPECT_EQ(ptn::match(one) | cases, 1);
  EXPECT_EQ(ptn::match(two) | cases, 2);
  EXPECT_EQ(ptn::match(ten) | cases, 10);
  EXPECT_EQ(ptn::match(other) | cases, 0);
}

TEST(LiteralPattern, LitCiMatchesAsciiCaseInsensitive) {
  std::string s = "HeLLo";
  int result    = ptn::match(s).when(ptn::lit_ci("hello") >> 1).otherwise(0);

  EXPECT_EQ(result, 1);
}
