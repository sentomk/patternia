#include <gtest/gtest.h>

#include <ptn/patternia.hpp>

using namespace ptn;

TEST(TerminalSemantics, OtherwiseUsedWhenNoCaseMatches) {
  int x = 3;

  int result = ptn::match(x).when(ptn::lit(1) >> 10).otherwise(7);

  EXPECT_EQ(result, 7);
}

TEST(TerminalSemantics, OtherwiseNotInvokedOnMatch) {
  int x              = 1;
  int fallback_calls = 0;

  int result = ptn::match(x).when(ptn::lit(1) >> 5).otherwise([&](int) {
    ++fallback_calls;
    return -1;
  });

  EXPECT_EQ(result, 5);
  EXPECT_EQ(fallback_calls, 0);
}

TEST(TerminalSemantics, EndWithWildcardReturnsFallbackCase) {
  int x = 2;

  const char *result = ptn::match(x)
                           .when(ptn::lit(1) >> "one")
                           .when(ptn::__ >> "other")
                           .end();

  EXPECT_STREQ(result, "other");
}

TEST(TerminalSemantics, FirstMatchingCaseWins) {
  int x = 9;

  int result =
      ptn::match(x).when(ptn::bind() >> 1).when(ptn::bind() >> 2).otherwise(0);

  EXPECT_EQ(result, 1);
}
