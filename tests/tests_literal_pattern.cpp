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

TEST(LiteralPattern, LitMatchesStringLiteral) {
  std::string s = "hello";
  int result    = ptn::match(s).when(ptn::lit("hello") >> 1).otherwise(0);

  EXPECT_EQ(result, 1);
}

TEST(LiteralPattern, LitStringOtherwiseFallback) {
  std::string s = "world";
  int result    = ptn::match(s).when(ptn::lit("hello") >> 1).otherwise(0);

  EXPECT_EQ(result, 0);
}

TEST(LiteralPattern, LitMatchesCompileTimeValue) {
  int x      = 5;
  int result = ptn::match(x).when(ptn::lit<5>() >> 42).otherwise(-1);

  EXPECT_EQ(result, 42);
}

TEST(LiteralPattern, LitCompileTimeOtherwiseFallback) {
  int x      = 3;
  int result = ptn::match(x).when(ptn::lit<5>() >> 42).otherwise(-1);

  EXPECT_EQ(result, -1);
}

TEST(LiteralPattern, LitSelectsSparseDenseCases) {
  auto cases = on(
      ptn::lit<1>() >> 1,
      ptn::lit<2>() >> 2,
      ptn::lit<10>() >> 10,
      __ >> 0
  );

  int one   = 1;
  int two   = 2;
  int ten   = 10;
  int other = 7;

  EXPECT_EQ(ptn::match(one) | cases, 1);
  EXPECT_EQ(ptn::match(two) | cases, 2);
  EXPECT_EQ(ptn::match(ten) | cases, 10);
  EXPECT_EQ(ptn::match(other) | cases, 0);
}

TEST(LiteralPattern, LitInlineOnSelectsDenseCases) {
  int one   = 1;
  int ten   = 10;
  int other = 7;

  int one_result = ptn::match(one)
                   | on(
                       ptn::lit<1>() >> 1,
                       ptn::lit<2>() >> 2,
                       ptn::lit<10>() >> 10,
                       __ >> 0
                   );
  int ten_result = ptn::match(ten)
                   | on(
                       ptn::lit<1>() >> 1,
                       ptn::lit<2>() >> 2,
                       ptn::lit<10>() >> 10,
                       __ >> 0
                   );
  int other_result = ptn::match(other)
                     | on(
                         ptn::lit<1>() >> 1,
                         ptn::lit<2>() >> 2,
                         ptn::lit<10>() >> 10,
                         __ >> 0
                     );

  EXPECT_EQ(one_result, 1);
  EXPECT_EQ(ten_result, 10);
  EXPECT_EQ(other_result, 0);
}

TEST(LiteralPattern, LitStaticOnFactorySelectsDenseCases) {
  int one   = 1;
  int ten   = 10;
  int other = 7;

  int one_result = ptn::match(one)
                   | ptn::static_on([] {
                       return on(
                           ptn::lit<1>() >> 1,
                           ptn::lit<2>() >> 2,
                           ptn::lit<10>() >> 10,
                           __ >> 0
                       );
                     });
  int ten_result = ptn::match(ten)
                   | ptn::static_on([] {
                       return on(
                           ptn::lit<1>() >> 1,
                           ptn::lit<2>() >> 2,
                           ptn::lit<10>() >> 10,
                           __ >> 0
                       );
                     });
  int other_result = ptn::match(other)
                     | ptn::static_on([] {
                         return on(
                             ptn::lit<1>() >> 1,
                             ptn::lit<2>() >> 2,
                             ptn::lit<10>() >> 10,
                             __ >> 0
                         );
                       });

  EXPECT_EQ(one_result, 1);
  EXPECT_EQ(ten_result, 10);
  EXPECT_EQ(other_result, 0);
}

TEST(LiteralPattern, LitPtnOnMacroSelectsDenseCases) {
  int one   = 1;
  int ten   = 10;
  int other = 7;

  int one_result = ptn::match(one)
                   | PTN_ON(
                       ptn::lit<1>() >> 1,
                       ptn::lit<2>() >> 2,
                       ptn::lit<10>() >> 10,
                       __ >> 0);
  int ten_result = ptn::match(ten)
                   | PTN_ON(
                       ptn::lit<1>() >> 1,
                       ptn::lit<2>() >> 2,
                       ptn::lit<10>() >> 10,
                       __ >> 0);
  int other_result = ptn::match(other)
                     | PTN_ON(
                         ptn::lit<1>() >> 1,
                         ptn::lit<2>() >> 2,
                         ptn::lit<10>() >> 10,
                         __ >> 0);

  EXPECT_EQ(one_result, 1);
  EXPECT_EQ(ten_result, 10);
  EXPECT_EQ(other_result, 0);
}

TEST(LiteralPattern, LitCiMatchesAsciiCaseInsensitive) {
  std::string s = "HeLLo";
  int result    = ptn::match(s).when(ptn::lit_ci("hello") >> 1).otherwise(0);

  EXPECT_EQ(result, 1);
}

