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

TEST(LiteralPattern, LitCiMatchesAsciiCaseInsensitive) {
  std::string s = "HeLLo";
  int result    = ptn::match(s).when(ptn::lit_ci("hello") >> 1).otherwise(0);

  EXPECT_EQ(result, 1);
}
