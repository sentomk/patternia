#include "gtest/gtest.h"
#include "ptn/patternia.hpp"

using namespace ptn;
using namespace ptn::pat::type;
using namespace ptn::core::dsl::ops;

//==============================================================
// type::is<T>
//==============================================================
TEST(TypePattern, ExactTypeMatch) {
  {
    int  x   = 42;
    auto out = match(x)
                   .when(pat::type::is<int> >> 1)
                   .when(pat::type::is<float> >> 2)
                   .otherwise(-1);
    EXPECT_EQ(out, 1);
  }

  {
    float x   = 3.14f;
    auto  out = match(x)
                   .when(pat::type::is<int> >> 1)
                   .when(pat::type::is<float> >> 2)
                   .otherwise(-1);
    EXPECT_EQ(out, 2);
  }
}

//==============================================================
// type::in<Ts...>
//==============================================================
TEST(TypePattern, InTypeSet) {
  {
    double x   = 0.5;
    auto   out = match(x)
                   .when(pat::type::in<int, float> >> 1)
                   .when(pat::type::in<float, double> >> 2)
                   .otherwise(-1);
    EXPECT_EQ(out, 2);
  }

  {
    long x = 5;
    auto out =
        match(x).when(pat::type::in<int, float, double> >> 1).otherwise(-1);
    EXPECT_EQ(out, -1);
  }
}

//==============================================================
// type::not_in<Ts...>
//==============================================================
TEST(TypePattern, NotInTypeSet) {
  {
    int  x   = 5;
    auto out = match(x).when(pat::type::not_in<int, float> >> 1).otherwise(2);
    EXPECT_EQ(out, 2);
  }

  {
    short x = 3;
    auto  out =
        match(x).when(pat::type::not_in<int, float, double> >> 7).otherwise(-1);
    EXPECT_EQ(out, 7);
  }
}

//==============================================================
// type::from<Tpl>
//==============================================================
#include <vector>
#include <optional>

TEST(TypePattern, FromTemplate) {
  {
    std::vector<int> v = {1, 2, 3};
    auto out = match(v).when(pat::type::from<std::vector> >> 100).otherwise(-1);
    EXPECT_EQ(out, 100);
  }

  {
    std::optional<int> v   = 123;
    auto               out = match(v)
                   .when(pat::type::from<std::vector> >> 1)
                   .when(pat::type::from<std::optional> >> 2)
                   .otherwise(-1);
    EXPECT_EQ(out, 2);
  }
}

//==============================================================
// mix：type + value patterns
//==============================================================
TEST(TypePattern, MixedWithValue) {
  int  x   = 42;
  auto out = match(x)
                 .when(pat::type::is<float> >> 1)
                 .when((pat::type::is<int> && pat::value::lit(42)) >> 2)
                 .otherwise(-1);
  EXPECT_EQ(out, 2);
}

//==============================================================
// first match wins
//==============================================================
TEST(TypePattern, FirstMatchWins) {
  double x   = 3.14;
  auto   out = match(x)
                 .when(pat::type::in<double, float> >> 10)
                 .when(pat::type::is<double> >> 20)
                 .otherwise(-1);
  EXPECT_EQ(out, 10);
}
