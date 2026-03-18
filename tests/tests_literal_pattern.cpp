#include <gtest/gtest.h>

#include <ptn/patternia.hpp>

#include <string>

using namespace ptn;

TEST(LiteralPattern, LitMatchesExpectedValue) {
  int x      = 5;
  int result = ptn::match(x) | ptn::on(ptn::lit(5) >> 42, ptn::__ >> -1);

  EXPECT_EQ(result, 42);
}

TEST(LiteralPattern, LitOtherwiseFallback) {
  int x      = 3;
  int result = ptn::match(x) | ptn::on(ptn::lit(5) >> 42, ptn::__ >> -1);

  EXPECT_EQ(result, -1);
}

TEST(LiteralPattern, LitMatchesStringLiteral) {
  std::string s = "hello";
  int result = ptn::match(s) | ptn::on(ptn::lit("hello") >> 1, ptn::__ >> 0);

  EXPECT_EQ(result, 1);
}

TEST(LiteralPattern, LitStringOtherwiseFallback) {
  std::string s = "world";
  int result = ptn::match(s) | ptn::on(ptn::lit("hello") >> 1, ptn::__ >> 0);

  EXPECT_EQ(result, 0);
}

TEST(LiteralPattern, LitMatchesCompileTimeValue) {
  int x      = 5;
  int result = ptn::match(x) | ptn::on(ptn::val<5> >> 42, ptn::__ >> -1);

  EXPECT_EQ(result, 42);
}

TEST(LiteralPattern, LitCompileTimeOtherwiseFallback) {
  int x      = 3;
  int result = ptn::match(x) | ptn::on(ptn::val<5> >> 42, ptn::__ >> -1);

  EXPECT_EQ(result, -1);
}

TEST(LiteralPattern, LitSelectsSparseDenseCases) {
  auto cases = on(
      ptn::val<1> >> 1,
      ptn::val<2> >> 2,
      ptn::val<10> >> 10,
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
                       ptn::val<1> >> 1,
                       ptn::val<2> >> 2,
                       ptn::val<10> >> 10,
                       __ >> 0
                   );
  int ten_result = ptn::match(ten)
                   | on(
                       ptn::val<1> >> 1,
                       ptn::val<2> >> 2,
                       ptn::val<10> >> 10,
                       __ >> 0
                   );
  int other_result = ptn::match(other)
                     | on(
                         ptn::val<1> >> 1,
                         ptn::val<2> >> 2,
                         ptn::val<10> >> 10,
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
                           ptn::val<1> >> 1,
                           ptn::val<2> >> 2,
                           ptn::val<10> >> 10,
                           __ >> 0
                       );
                     });
  int ten_result = ptn::match(ten)
                   | ptn::static_on([] {
                       return on(
                           ptn::val<1> >> 1,
                           ptn::val<2> >> 2,
                           ptn::val<10> >> 10,
                           __ >> 0
                       );
                     });
  int other_result = ptn::match(other)
                     | ptn::static_on([] {
                         return on(
                             ptn::val<1> >> 1,
                             ptn::val<2> >> 2,
                             ptn::val<10> >> 10,
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
                       ptn::val<1> >> 1,
                       ptn::val<2> >> 2,
                       ptn::val<10> >> 10,
                       __ >> 0);
  int ten_result = ptn::match(ten)
                   | PTN_ON(
                       ptn::val<1> >> 1,
                       ptn::val<2> >> 2,
                       ptn::val<10> >> 10,
                       __ >> 0);
  int other_result = ptn::match(other)
                     | PTN_ON(
                         ptn::val<1> >> 1,
                         ptn::val<2> >> 2,
                         ptn::val<10> >> 10,
                         __ >> 0);

  EXPECT_EQ(one_result, 1);
  EXPECT_EQ(ten_result, 10);
  EXPECT_EQ(other_result, 0);
}

TEST(LiteralPattern, LitCiMatchesAsciiCaseInsensitive) {
  std::string s = "HeLLo";
  int result = ptn::match(s) | ptn::on(ptn::lit_ci("hello") >> 1, ptn::__ >> 0);

  EXPECT_EQ(result, 1);
}

#if (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L) || __cplusplus >= 202002L
TEST(LiteralPattern, ValSupportsFloatingPointInCpp20) {
  double x   = 3.25;
  int    hit = ptn::match(x) | ptn::on(ptn::val<3.25> >> 7, ptn::__ >> -1);

  EXPECT_EQ(hit, 7);
}
#endif

