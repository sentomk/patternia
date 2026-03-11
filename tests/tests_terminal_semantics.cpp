#include <gtest/gtest.h>

#include <ptn/patternia.hpp>

using namespace ptn;

TEST(TerminalSemantics, WildcardUsedWhenNoCaseMatches) {
  int x = 3;

  int result = ptn::match(x) | ptn::on(ptn::lit(1) >> 10, ptn::__ >> 7);

  EXPECT_EQ(result, 7);
}

TEST(TerminalSemantics, WildcardSkippedWhenCaseMatches) {
  int x              = 1;
  int fallback_calls = 0;

  int result = ptn::match(x)
               | ptn::on(ptn::lit(1) >> 5,
                         ptn::__ >> [&] {
                           ++fallback_calls;
                           return -1;
                         });

  EXPECT_EQ(result, 5);
  EXPECT_EQ(fallback_calls, 0);
}

TEST(TerminalSemantics, WildcardReturnsFallbackCase) {
  int x = 2;

  const char *result = ptn::match(x)
                       | ptn::on(ptn::lit(1) >> "one", ptn::__ >> "other");

  EXPECT_STREQ(result, "other");
}

TEST(TerminalSemantics, FirstMatchingCaseWins) {
  int x = 9;

  int result = ptn::match(x)
               | ptn::on(ptn::bind() >> 1, ptn::bind() >> 2, ptn::__ >> 0);

  EXPECT_EQ(result, 1);
}

