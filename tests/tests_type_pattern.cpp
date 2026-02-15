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
