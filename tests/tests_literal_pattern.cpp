#include "gtest/gtest.h"
#include "ptn/patternia.hpp"

#include <cmath>
#include <optional>
#include <string>
#include <string_view>

using namespace ptn;
using namespace ptn::pat::value;
using namespace ptn::core::dsl::ops;

TEST(LiteralPattern, IntExactMatch) {
  int  x   = 42;
  auto out = match(x)
                 .when(lit(0) >> [](int) { return std::string{"zero"}; })
                 .when(lit(42) >> [](int) { return std::string{"answer"}; })
                 .otherwise([](int) { return std::string{"other"}; });
  EXPECT_EQ(out, "answer");
}

TEST(LiteralPattern, CStringVsString_CaseInsensitive) {
  std::string s   = "Ok";
  auto        out = match(s)
                 .when(lit("OK") >> "cs")
                 .when(lit_ci("OK") >> "ci")
                 .otherwise("other");
  EXPECT_EQ(out, "ci");
}

TEST(LiteralPattern, IntNoMatchFallsToOtherwise) {
  int  x   = -1;
  auto out = match(x)
                 .when(lit(0) >> [](int) { return std::string{"zero"}; })
                 .when(lit(42) >> [](int) { return std::string{"answer"}; })
                 .otherwise([](int) { return std::string{"other"}; });
  EXPECT_EQ(out, "other");
}

TEST(LiteralPattern, HeterogeneousEquality_IntVsLongLong) {
  int  x   = 42;
  auto out = match(x)
                 .when(lit(42LL) >> [](int) { return std::string{"ok"}; })
                 .otherwise([](int) { return std::string{"no"}; });
  EXPECT_EQ(out, "ok");
}

enum class Color {
  Red   = 1,
  Green = 2,
  Blue  = 3
};

TEST(LiteralPattern, EnumExactMatch) {
  Color c = Color::Green;
  auto  out =
      match(c)
          .when(lit(Color::Red) >> [](auto) { return std::string{"R"}; })
          .when(lit(Color::Green) >> [](auto) { return std::string{"G"}; })
          .otherwise([](auto) { return std::string{"X"}; });
  EXPECT_EQ(out, "G");
}

TEST(LiteralPattern, StringTypes_CaseSensitive) {
  std::string_view sv  = "hello";
  auto             out = match(sv)
                 .when(lit("HELLO") >> [](auto) { return std::string{"cs1"}; })
                 .when(
                     lit(std::string{"hello"}) >>
                     [](auto) { return std::string{"cs2"}; })
                 .otherwise([](auto) { return std::string{"other"}; });
  EXPECT_EQ(out, "cs2");
}

TEST(LiteralPattern, StringTypes_CaseInsensitive_Mixed) {
  std::string s = "HeLLo";
  auto        out =
      match(s)
          .when(lit_ci("hello") >> [](auto) { return std::string{"ci"}; })
          .otherwise([](auto) { return std::string{"other"}; });
  EXPECT_EQ(out, "ci");
}

struct approx_equal {
  double eps{1e-6};
  bool   operator()(double x, double y) const {
    return std::fabs(x - y) <= eps;
  }
};

TEST(LiteralPattern, DoubleApproxEqual_CustomComparator) {
  double x = 3.141592;
  auto   out =
      match(x)
          .when(
              ptn::pat::value::detail::literal_pattern<double, approx_equal>{
                  3.141593, approx_equal{1e-5}} >>
              [](double) { return std::string{"ok"}; })
          .otherwise([](double) { return std::string{"no"}; });
  EXPECT_EQ(out, "ok");
}

struct Point {
  int x{};
  int y{};
};

struct eq_by_x {
  template <class A, class B>
  bool operator()(const A &a, const B &b) const {
    return a.x == b.x;
  }
};

TEST(LiteralPattern, UserType_CustomComparator_ByField) {
  Point p{10, 20};
  auto  out = match(p)
                 .when(
                     ptn::pat::value::detail::literal_pattern<Point, eq_by_x>{
                         Point{10, 999}, eq_by_x{}} >>
                     [](const Point &) { return std::string{"x"}; })
                 .otherwise([](const Point &) { return std::string{"other"}; });
  EXPECT_EQ(out, "x");
}

TEST(LiteralPattern, FirstMatchWins_OrderMatters) {
  int  x   = 7;
  auto out = match(x)
                 // 注意：lambda 直接可用作 pattern
                 .when(
                     pred([](int v) { return v > 0; }) >>
                     [](int) { return std::string{"pos"}; })
                 .when(lit(7) >> [](int) { return std::string{"exact-7"}; })
                 .otherwise([](int) { return std::string{"other"}; });
  EXPECT_EQ(out, "pos");
}

TEST(LiteralPattern, OptionalValueMatch) {
  std::optional<int> oi  = 42;
  auto               out = match(oi)
                 .when(
                     lit(std::optional<int>{}) >>
                     [](auto) { return std::string{"empty"}; })
                 .when(
                     lit(std::optional<int>{42}) >>
                     [](auto) { return std::string{"some42"}; })
                 .otherwise([](auto) { return std::string{"other"}; });
  EXPECT_EQ(out, "some42");
}

TEST(LiteralPattern, DoubleNaN_NoMatchWithDefaultEqual) {
  double x   = std::numeric_limits<double>::quiet_NaN();
  auto   out = match(x)
                 .when(
                     lit(std::numeric_limits<double>::quiet_NaN()) >>
                     [](double) { return std::string{"nan"}; })
                 .otherwise([](double) { return std::string{"other"}; });
  EXPECT_EQ(out, "other");
}

TEST(LiteralPattern, CStringLiteralMatchesStringView) {
  const char *cstr = "abc";
  auto        out  = match(std::string_view{"abc"})
                 .when(lit(cstr) >> [](auto) { return std::string{"hit"}; })
                 .otherwise([](auto) { return std::string{"miss"}; });
  EXPECT_EQ(out, "hit");
}

TEST(LiteralPattern, HeterogeneousEquality_Int64VsInt32) {
  long long big = 1234567890LL;
  auto      out =
      match(big)
          .when(lit(1234567890) >> [](auto) { return std::string{"ok"}; })
          .otherwise([](auto) { return std::string{"no"}; });
  EXPECT_EQ(out, "ok");
}
