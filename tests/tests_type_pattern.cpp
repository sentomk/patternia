#include <gtest/gtest.h>

#include <ptn/patternia.hpp>

#include <string>
#include <variant>

using namespace ptn;

TEST(TypePattern, TypeIsAndTypeAs) {
  std::variant<int, std::string> v = std::string("patternia");

  int result = ptn::match(v)
                   .when(ptn::type::is<int>() >> [] { return -1; })
                   .when(ptn::type::as<std::string>() >>
                         [](const std::string &s) {
                           return static_cast<int>(s.size());
                         })
                   .otherwise(0);

  EXPECT_EQ(result, 9);
}

TEST(TypePattern, AltByIndex) {
  std::variant<int, std::string> v = 42;

  const char *result = ptn::match(v)
                           .when(ptn::alt<0>() >> "int")
                           .when(ptn::alt<1>() >> "string")
                           .when(ptn::__ >> "other")
                           .end();

  EXPECT_STREQ(result, "int");
}

TEST(TypePattern, SimpleVariantDispatchPreservesFirstMatchWins) {
  std::variant<int, std::string> v = 42;

  int hit_count = 0;
  int result    = ptn::match(v)
                   .when(ptn::type::is<int>() >> [&] {
                     ++hit_count;
                     return 1;
                   })
                   .when(ptn::type::is<int>() >> [&] {
                     ++hit_count;
                     return 2;
                   })
                   .when(ptn::__ >> [&] {
                     ++hit_count;
                     return 3;
                   })
                   .end();

  EXPECT_EQ(result, 1);
  EXPECT_EQ(hit_count, 1);
}

TEST(TypePattern, SimpleVariantDispatchFallsBackToOtherwise) {
  std::variant<int, std::string> v = std::string("patternia");

  int result = ptn::match(v)
                   .when(ptn::type::is<int>() >> 1)
                   .otherwise(99);

  EXPECT_EQ(result, 99);
}

TEST(TypePattern, SimpleVariantDispatchUnlistedAltFallsToWildcard) {
  std::variant<int, std::string, double> v = 3.14;

  int wildcard_hits = 0;
  int int_hits      = 0;
  int str_hits      = 0;

  int result = ptn::match(v)
                   .when(ptn::type::is<int>() >> [&] {
                     ++int_hits;
                     return 1;
                   })
                   .when(ptn::type::is<std::string>() >> [&] {
                     ++str_hits;
                     return 2;
                   })
                   .when(ptn::__ >> [&] {
                     ++wildcard_hits;
                     return 7;
                   })
                   .end();

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
                   .when(ptn::type::as<int>()[ptn::_ > 100] >> [&](int) {
                     ++guarded_hits;
                     return 10;
                   })
                   .when(ptn::type::is<int>() >> [&] {
                     ++simple_hits;
                     return 1;
                   })
                   .when(ptn::__ >> [] { return 0; })
                   .end();

  EXPECT_EQ(result, 1);
  EXPECT_EQ(guarded_hits, 0);
  EXPECT_EQ(simple_hits, 1);
}

TEST(TypePattern, MixedVariantGuardedCaseWinsWhenPredicateTrue) {
  std::variant<int, std::string> v = std::string("patternia");
  auto long_string = [](const std::string &s) { return s.size() > 4; };

  int guarded_hits = 0;
  int simple_hits  = 0;

  int result = ptn::match(v)
                   .when(ptn::type::as<std::string>()[long_string] >>
                         [&](const std::string &) {
                           ++guarded_hits;
                           return 20;
                         })
                   .when(ptn::type::is<std::string>() >> [&] {
                     ++simple_hits;
                     return 2;
                   })
                   .when(ptn::__ >> [] { return 0; })
                   .end();

  EXPECT_EQ(result, 20);
  EXPECT_EQ(guarded_hits, 1);
  EXPECT_EQ(simple_hits, 0);
}
