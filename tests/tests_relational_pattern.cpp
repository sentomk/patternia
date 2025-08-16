#include "gtest/gtest.h"
#include "ptn/patternia.hpp"

#include <string>
#include <string_view>
#include <limits>

using namespace ptn;
using namespace ptn::patterns;

TEST(RelationalPattern, IntBasicComparisons) {
  int  x   = 10;
  auto out = match(x)
                 .with(lt(0), [](int) { return std::string{"neg"}; })
                 .with(le(10), [](int) { return std::string{"<=10"}; })
                 .with(gt(10), [](int) { return std::string{">10"}; })
                 .otherwise([](int) { return std::string{"other"}; });
  EXPECT_EQ(out, "<=10");

  x   = -5;
  out = match(x)
            .with(lt(0), [](int) { return std::string{"neg"}; })
            .otherwise([](int) { return std::string{"other"}; });
  EXPECT_EQ(out, "neg");

  x   = 11;
  out = match(x)
            .with(ge(11), [](int) { return std::string{">=11"}; })
            .otherwise([](int) { return std::string{"other"}; });
  EXPECT_EQ(out, ">=11");

  x   = 42;
  out = match(x)
            .with(eq(42), [](int) { return std::string{"eq"}; })
            .with(ne(42), [](int) { return std::string{"ne"}; })
            .otherwise([](int) { return std::string{"other"}; });
  EXPECT_EQ(out, "eq");
}

TEST(RelationalPattern, Boundaries) {
  int x = 0;

  auto out = match(x)
                 .with(lt(0), [](int) { return std::string{"lt0"}; })
                 .with(le(0), [](int) { return std::string{"le0"}; })
                 .otherwise([](int) { return std::string{"other"}; });
  EXPECT_EQ(out, "le0");

  out = match(x)
            .with(gt(0), [](int) { return std::string{"gt0"}; })
            .with(ge(0), [](int) { return std::string{"ge0"}; })
            .otherwise([](int) { return std::string{"other"}; });
  EXPECT_EQ(out, "ge0");
}

TEST(RelationalPattern, Heterogeneous_IntVsLongLong) {
  int  x   = 100;
  auto out = match(x)
                 .with(lt(100LL), [](int) { return std::string{"lt"}; })
                 .with(ge(100LL), [](int) { return std::string{"ge"}; })
                 .otherwise([](int) { return std::string{"other"}; });
  EXPECT_EQ(out, "ge");
}

TEST(RelationalPattern, Heterogeneous_DoubleVsInt) {
  double x   = 9.999;
  auto   out = match(x)
                 .with(lt(10), [](double) { return std::string{"lt10"}; })
                 .with(ge(10), [](double) { return std::string{"ge10"}; })
                 .otherwise([](double) { return std::string{"other"}; });
  EXPECT_EQ(out, "lt10");

  x   = 10.0;
  out = match(x)
            .with(gt(10), [](double) { return std::string{"gt10"}; })
            .with(ge(10), [](double) { return std::string{"ge10"}; })
            .otherwise([](double) { return std::string{"other"}; });
  EXPECT_EQ(out, "ge10");
}

TEST(RelationalPattern, StringLexicographic) {
  std::string_view sv = "bbb";
  auto             out =
      match(sv)
          .with(lt("aaa"), [](auto) { return std::string{"<aaa"}; })
          .with(
              ge(std::string{"bbb"}), [](auto) { return std::string{">=bbb"}; })
          .otherwise([](auto) { return std::string{"other"}; });
  EXPECT_EQ(out, ">=bbb");

  std::string s = "abc";
  out           = match(s)
            .with(
                gt(std::string_view{"zzz"}),
                [](auto) { return std::string{">zzz"}; })
            .with(le("abc"), [](auto) { return std::string{"<=abc"}; })
            .otherwise([](auto) { return std::string{"other"}; });
  EXPECT_EQ(out, "<=abc");
}

TEST(RelationalPattern, FirstMatchWins) {
  int  x   = 5;
  auto out = match(x)
                 .with(gt(0), [](int) { return std::string{"pos"}; })
                 .with(ge(5), [](int) { return std::string{"ge5"}; })
                 .with(eq(5), [](int) { return std::string{"eq5"}; })
                 .otherwise([](int) { return std::string{"other"}; });
  EXPECT_EQ(out, "pos");
}

TEST(RelationalPattern, MixWithValuePattern) {
  int  x   = 10;
  auto out = match(x)
                 .with(lt(10), [](int) { return std::string{"lt10"}; })
                 .with(value(10), [](int) { return std::string{"eq10"}; })
                 .with(ge(11), [](int) { return std::string{">=11"}; })
                 .otherwise([](int) { return std::string{"other"}; });
  EXPECT_EQ(out, "eq10");
}

TEST(RelationalPattern, ExtremeValues) {
  int  x   = std::numeric_limits<int>::max();
  auto out = match(x)
                 .with(
                     ge(std::numeric_limits<int>::max()),
                     [](int) { return std::string{"ge_max"}; })
                 .otherwise([](int) { return std::string{"other"}; });
  EXPECT_EQ(out, "ge_max");

  x   = std::numeric_limits<int>::min();
  out = match(x)
            .with(
                le(std::numeric_limits<int>::min()),
                [](int) { return std::string{"le_min"}; })
            .otherwise([](int) { return std::string{"other"}; });
  EXPECT_EQ(out, "le_min");
}

TEST(RelationalPattern, Interval_OpenClosed_0_10) {
  int  x   = 10;
  auto out = match(x)
                 .with(
                     [](int v) { return gt(0)(v) && le(10)(v); },
                     [](int) { return std::string{"(0,10]"}; })
                 .otherwise([](int) { return std::string{"other"}; });
  EXPECT_EQ(out, "(0,10]");

  x   = 1;
  out = match(x)
            .with(
                [](int v) { return gt(0)(v) && le(10)(v); },
                [](int) { return std::string{"(0,10]"}; })
            .otherwise([](int) { return std::string{"other"}; });
  EXPECT_EQ(out, "(0,10]");
}

TEST(RelationalPattern, DoubleRange_Inclusive) {
  double x   = 1.0;
  auto   out = match(x)
                 .with(
                     [](double v) { return ge(1.0)(v) && le(2.0)(v); },
                     [](double) { return std::string{"[1,2]"}; })
                 .otherwise([](double) { return std::string{"other"}; });
  EXPECT_EQ(out, "[1,2]");

  x   = 2.0;
  out = match(x)
            .with(
                [](double v) { return ge(1.0)(v) && le(2.0)(v); },
                [](double) { return std::string{"[1,2]"}; })
            .otherwise([](double) { return std::string{"other"}; });
  EXPECT_EQ(out, "[1,2]");
}
