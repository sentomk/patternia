#include <gtest/gtest.h>

#include <ptn/patternia.hpp>

#include <string>
#include <variant>

using namespace ptn;

TEST(TypePattern, TypeIsAndTypeAs) {
  std::variant<int, std::string> v = std::string("patternia");

  int result = ptn::match(v)
               | ptn::on(ptn::is<int>() >> [] { return -1; },
                         ptn::$(ptn::is<std::string>()) >>
                             [](const std::string &s) {
                               return static_cast<int>(s.size());
                             },
                         ptn::__ >> 0);

  EXPECT_EQ(result, 9);
}

TEST(TypePattern, AltByIndex) {
  std::variant<int, std::string> v = 42;

  const char *result = ptn::match(v)
                       | ptn::on(ptn::alt<0>() >> "int",
                                 ptn::alt<1>() >> "string",
                                 ptn::__ >> "other");

  EXPECT_STREQ(result, "int");
}

TEST(TypePattern, SimpleVariantDispatchPreservesFirstMatchWins) {
  std::variant<int, std::string> v = 42;

  int hit_count = 0;
  int result = ptn::match(v)
               | ptn::on(ptn::is<int>() >>
                             [&] {
                               ++hit_count;
                               return 1;
                             },
                         ptn::is<int>() >>
                             [&] {
                               ++hit_count;
                               return 2;
                             },
                         ptn::__ >>
                             [&] {
                               ++hit_count;
                               return 3;
                             });

  EXPECT_EQ(result, 1);
  EXPECT_EQ(hit_count, 1);
}

TEST(TypePattern, SimpleVariantDispatchFallsBackToWildcard) {
  std::variant<int, std::string> v = std::string("patternia");

  int result = ptn::match(v) | ptn::on(ptn::is<int>() >> 1, ptn::__ >> 99);

  EXPECT_EQ(result, 99);
}

TEST(TypePattern, SimpleVariantDispatchUnlistedAltFallsToWildcard) {
  std::variant<int, std::string, double> v = 3.14;

  int wildcard_hits = 0;
  int int_hits      = 0;
  int str_hits      = 0;

  int result = ptn::match(v)
               | ptn::on(ptn::is<int>() >>
                             [&] {
                               ++int_hits;
                               return 1;
                             },
                         ptn::is<std::string>() >>
                             [&] {
                               ++str_hits;
                               return 2;
                             },
                         ptn::__ >>
                             [&] {
                               ++wildcard_hits;
                               return 7;
                             });

  EXPECT_EQ(result, 7);
  EXPECT_EQ(wildcard_hits, 1);
  EXPECT_EQ(int_hits, 0);
  EXPECT_EQ(str_hits, 0);
}

TEST(TypePattern, MixedVariantGuardedFallsThroughToSimpleCase) {
  std::variant<int, std::string> v = 42;

  int guarded_hits = 0;
  int simple_hits  = 0;

  int result = ptn::match(v)
               | ptn::on(ptn::$(ptn::is<int>())[ptn::_0 > 100] >>
                             [&](int) {
                               ++guarded_hits;
                               return 10;
                             },
                         ptn::is<int>() >>
                             [&] {
                               ++simple_hits;
                               return 1;
                             },
                         ptn::__ >> [] { return 0; });

  EXPECT_EQ(result, 1);
  EXPECT_EQ(guarded_hits, 0);
  EXPECT_EQ(simple_hits, 1);
}

TEST(TypePattern, MixedVariantGuardedCaseWinsWhenPredicateTrue) {
  std::variant<int, std::string> v = std::string("patternia");
  auto long_string                 = [](const std::string &s) {
    return s.size() > 4;
  };

  int guarded_hits = 0;
  int simple_hits  = 0;

  int result = ptn::match(v)
               | ptn::on(ptn::$(ptn::is<std::string>())[long_string]
                             >>
                             [&](const std::string &) {
                               ++guarded_hits;
                               return 20;
                             },
                         ptn::is<std::string>() >>
                             [&] {
                               ++simple_hits;
                               return 2;
                             },
                         ptn::__ >> [] { return 0; });

  EXPECT_EQ(result, 20);
  EXPECT_EQ(guarded_hits, 1);
  EXPECT_EQ(simple_hits, 0);
}
