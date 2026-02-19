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
