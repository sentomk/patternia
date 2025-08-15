#include "gtest/gtest.h"
#include "ptn/patternia.hpp"

#include <cmath>
#include <optional>
#include <string>
#include <string_view>

using namespace ptn;
using namespace ptn::patterns;

TEST(ValuePattern, IntExactMatch) {
  int  x   = 42;
  auto out = match(x)
                 .with(value(0), [](int) { return std::string{"zero"}; })
                 .with(value(42), [](int) { return std::string{"answer"}; })
                 .otherwise([](int) { return std::string{"other"}; });
  EXPECT_EQ(out, "answer");
}

TEST(ValuePattern, CStringVsString_CaseInsensitive) {
  std::string s   = "Ok";
  auto        out = match(s)
                 .with(value("OK"), [](auto) { return std::string{"cs"}; })
                 .with(ci_value("OK"), [](auto) { return std::string{"ci"}; })
                 .otherwise([](auto) { return std::string{"other"}; });
  EXPECT_EQ(out, "ci");
}

TEST(ValuePattern, IntNoMatchFallsToOtherwise) {
  int  x   = -1;
  auto out = match(x)
                 .with(value(0), [](int) { return std::string{"zero"}; })
                 .with(value(42), [](int) { return std::string{"answer"}; })
                 .otherwise([](int) { return std::string{"other"}; });
  EXPECT_EQ(out, "other");
}

TEST(ValuePattern, HeterogeneousEquality_IntVsLongLong) {
  int  x   = 42;
  auto out = match(x).with(value(42LL), [](int) { return std::string{"ok"}; }).otherwise([](int) {
    return std::string{"no"};
  });
  EXPECT_EQ(out, "ok");
}

enum class Color {
  Red   = 1,
  Green = 2,
  Blue  = 3
};
TEST(ValuePattern, EnumExactMatch) {
  Color c   = Color::Green;
  auto  out = match(c)
                 .with(value(Color::Red), [](auto) { return std::string{"R"}; })
                 .with(value(Color::Green), [](auto) { return std::string{"G"}; })
                 .otherwise([](auto) { return std::string{"X"}; });
  EXPECT_EQ(out, "G");
}

TEST(ValuePattern, StringTypes_CaseSensitive) {
  std::string_view sv  = "hello";
  auto             out = match(sv)
                 .with(value("HELLO"), [](auto) { return std::string{"cs1"}; })
                 .with(value(std::string{"hello"}), [](auto) { return std::string{"cs2"}; })
                 .otherwise([](auto) { return std::string{"other"}; });
  EXPECT_EQ(out, "cs2");
}

TEST(ValuePattern, StringTypes_CaseInsensitive_Mixed) {
  std::string s = "HeLLo";
  auto        out =
      match(s).with(ci_value("hello"), [](auto) { return std::string{"ci"}; }).otherwise([](auto) {
        return std::string{"other"};
      });
  EXPECT_EQ(out, "ci");
}

struct approx_equal {
  double eps{1e-6};
  bool   operator()(double x, double y) const {
    return std::fabs(x - y) <= eps;
  }
};

TEST(ValuePattern, DoubleApproxEqual_CustomComparator) {
  double x   = 3.141592;
  auto   out = match(x)
                 .with(
                     value_pattern<double, approx_equal>{3.141593, approx_equal{1e-5}},
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

TEST(ValuePattern, UserType_CustomComparator_ByField) {
  Point p{10, 20};
  auto  out = match(p)
                 .with(
                     value_pattern<Point, eq_by_x>{Point{10, 999}, eq_by_x{}},
                     [](const Point &) { return std::string{"x"}; })
                 .otherwise([](const Point &) { return std::string{"other"}; });
  EXPECT_EQ(out, "x");
}

TEST(ValuePattern, FirstMatchWins_OrderMatters) {
  int  x   = 7;
  auto out = match(x)
                 .with([](int v) { return v > 0; }, [](int) { return std::string{"pos"}; })
                 .with(value(7), [](int) { return std::string{"exact-7"}; })
                 .otherwise([](int) { return std::string{"other"}; });
  EXPECT_EQ(out, "pos");
}

TEST(ValuePattern, OptionalValueMatch) {
  std::optional<int> oi  = 42;
  auto               out = match(oi)
                 .with(value(std::optional<int>{}), [](auto) { return std::string{"empty"}; })
                 .with(value(std::optional<int>{42}), [](auto) { return std::string{"some42"}; })
                 .otherwise([](auto) { return std::string{"other"}; });
  EXPECT_EQ(out, "some42");
}

TEST(ValuePattern, DoubleNaN_NoMatchWithDefaultEqual) {
  double x   = std::numeric_limits<double>::quiet_NaN();
  auto   out = match(x)
                 .with(
                     value(std::numeric_limits<double>::quiet_NaN()),
                     [](double) { return std::string{"nan"}; })
                 .otherwise([](double) { return std::string{"other"}; });
  EXPECT_EQ(out, "other");
}

TEST(ValuePattern, CStringLiteralMatchesStringView) {
  const char *cstr = "abc";
  auto        out  = match(std::string_view{"abc"})
                 .with(value(cstr), [](auto) { return std::string{"hit"}; })
                 .otherwise([](auto) { return std::string{"miss"}; });
  EXPECT_EQ(out, "hit");
}

TEST(ValuePattern, HeterogeneousEquality_Int64VsInt32) {
  long long big = 1234567890LL;
  auto      out = match(big)
                 .with(value(1234567890), [](auto) { return std::string{"ok"}; })
                 .otherwise([](auto) { return std::string{"no"}; });
  EXPECT_EQ(out, "ok");
}
