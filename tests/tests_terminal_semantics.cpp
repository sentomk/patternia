#include <gtest/gtest.h>

#include <ptn/patternia.hpp>

using namespace ptn;

TEST(TerminalSemantics, WildcardUsedWhenNoCaseMatches) {
  int x = 3;

  int result = ptn::match(x)
               | ptn::on(ptn::lit(1) >> 10, ptn::_ >> 7);

  EXPECT_EQ(result, 7);
}

TEST(TerminalSemantics, WildcardSkippedWhenCaseMatches) {
  int x              = 1;
  int fallback_calls = 0;

  int result = ptn::match(x)
               | ptn::on(
                   ptn::lit(1) >> 5, ptn::_ >> [&] {
                     ++fallback_calls;
                     return -1;
                   });

  EXPECT_EQ(result, 5);
  EXPECT_EQ(fallback_calls, 0);
}

TEST(TerminalSemantics, WildcardReturnsFallbackCase) {
  int x = 2;

  const char *result = ptn::match(x)
                       | ptn::on(ptn::lit(1) >> "one",
                                 ptn::_ >> "other");

  EXPECT_STREQ(result, "other");
}

TEST(TerminalSemantics, FirstMatchingCaseWins) {
  int x = 9;

  int result = ptn::match(x)
               | ptn::on(ptn::$ >> 1, ptn::$ >> 2, ptn::_ >> 0);

  EXPECT_EQ(result, 1);
}
