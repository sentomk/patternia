#include <gtest/gtest.h>

#include <ptn/patternia.hpp>

#include <string>
#include <tuple>
#include <type_traits>
#include <variant>

using namespace ptn;

namespace {
  struct Point {
    int x;
    int y;
  };

  constexpr auto point = has<&Point::x, &Point::y>;
  auto [px, py]        = point;

  // This pattern owns its bound string in a temporary tuple rather
  // than borrowing storage from the subject.
  struct materialized_string
      : pat::base::pattern_base<materialized_string> {
    bool match(int) const {
      return true;
    }

    auto bind(int size) const {
      return std::make_tuple(std::string(size, 'x'));
    }
  };
} // namespace

namespace ptn::pat::base {
  template <typename Subject>
  struct binding_args<materialized_string, Subject> {
    using type = std::tuple<std::string>;
  };
} // namespace ptn::pat::base

TEST(ExpressionHandler, MaterializesResultBeforeBindingsExpire) {
  int  n             = 128;
  auto bound_case    = materialized_string{} >> _;
  using handler_type = decltype(bound_case.handler);
  static_assert(std::is_same_v<
                std::invoke_result_t<handler_type, std::string &>,
                std::string>);
  EXPECT_EQ(match(n) | on(bound_case, _ >> std::string{}),
            std::string(n, 'x'));
}

TEST(ExpressionHandler, ArithmeticReturnsComputedValue) {
  int  n      = 4;
  auto result = match(n) | on($ >> _ * 2 + 1);
  static_assert(std::is_same_v<decltype(result), int>);
  EXPECT_EQ(result, 9);
  EXPECT_EQ(match(n) | on($() >> (10 - _) / 2 + _ % 3), 4);
  EXPECT_DOUBLE_EQ(match(n) | on($ >> _ / 2.0), 2.0);
}

TEST(ExpressionHandler, BarePlaceholderAndStoredExpression) {
  const std::string text = "hello";
  EXPECT_EQ(match(text) | on($ >> _), text);
  int  offset     = 3;
  auto expression = _ + offset;
  offset          = 100;
  EXPECT_EQ(offset, 100);
  auto cases = on($ >> expression);
  int  n     = 4;
  EXPECT_EQ(match(n) | cases, 7);
  n = 6;
  EXPECT_EQ(match(n) | cases, 9);
}

TEST(ExpressionHandler, StructuralNamesFollowMemberOrder) {
  Point p{3, 4};
  EXPECT_EQ(match(p) | on($(point) >> px * px + py * py, _ >> 0),
            25);
  EXPECT_EQ(match(p)
                | on($(has<&Point::y, _ign, &Point::x>) >> px - py,
                     _ >> 0),
            -1);
  EXPECT_EQ(match(p) | on($(point) >> px, _ >> 0), 3);
}

TEST(ExpressionHandler, GuardAndHandlerResolveIndependently) {
  auto  cases = on($(point)[px > 0] >> px - py,
                  $(point) >> px * px + py * py,
                  _ >> 0);
  Point p{3, 4};
  EXPECT_EQ(match(p) | cases, -1);
  p.x = -3;
  EXPECT_EQ(match(p) | cases, 25);
}

TEST(ExpressionHandler, ChainedGuardsKeepMemberResolution) {
  Point p{3, 4};
  EXPECT_EQ(
      match(p) | on($(point)[px > 0][py > 0] >> py - px, _ >> 0), 1);
}

TEST(ExpressionHandler, UnaryPlusAndMinus) {
  int n = 4;
  EXPECT_EQ(match(n) | on($ >> -_), -4);
  EXPECT_EQ(match(n) | on($ >> +_), 4);
  Point p{3, 4};
  EXPECT_EQ(match(p) | on($(point) >> -(px + py) + +py, _ >> 0), -3);
  EXPECT_EQ(match(p) | on($(point)[-px < 0] >> -py, _ >> 0), -4);
}

TEST(ExpressionHandler, UnaryExpressionsCaptureRuntimeOperands) {
  Point p{3, 4};
  for (int offset : {2, 5}) {
    auto      cases = on($(point) >> -(px + offset) + +(py - offset),
                    _ >> 0);
    const int captured = offset;
    offset             = 100;
    EXPECT_EQ(offset, 100);
    EXPECT_EQ(match(p) | cases,
              -(p.x + captured) + (p.y - captured));
  }
}

TEST(ExpressionHandler, ComparisonsReturnBool) {
  int  n      = 4;
  auto result = match(n) | on($ >> (_ > 3));
  static_assert(std::is_same_v<decltype(result), bool>);
  EXPECT_TRUE(result);
  EXPECT_TRUE(match(n) | on($ >> (_ >= 4)));
  EXPECT_TRUE(match(n) | on($ >> (_ <= 4)));
  EXPECT_FALSE(match(n) | on($ >> (_ < 4)));
  EXPECT_TRUE(match(n) | on($ >> (_ == 4)));
  EXPECT_FALSE(match(n) | on($ >> (_ != 4)));
  Point p{3, 4};
  EXPECT_TRUE(match(p) | on($(point) >> (px < py), _ >> false));
}

TEST(ExpressionHandler, LogicalExpressionsShortCircuit) {
  int  calls = 0;
  auto probe = [&calls](int) {
    ++calls;
    return true;
  };
  auto cases = on($ >> ((_ > 0 && probe) || _ == -1));
  int  n     = -1;
  EXPECT_TRUE(match(n) | cases);
  EXPECT_EQ(calls, 0);
  n = -2;
  EXPECT_FALSE(match(n) | cases);
  EXPECT_EQ(calls, 0);
  n = 2;
  EXPECT_TRUE(match(n) | cases);
  EXPECT_EQ(calls, 1);
  auto skip_probe = on($ >> (_ > 0 || probe));
  EXPECT_TRUE(match(n) | skip_probe);
  EXPECT_EQ(calls, 1);
}

TEST(ExpressionHandler, StructuralLogicalExpressions) {
  Point p{3, 4};
  EXPECT_TRUE(
      match(p)
      | on($(point) >> ((px > 0 && py > 0) || px == 0), _ >> false));
  p.x = -1;
  EXPECT_FALSE(match(p)
               | on($(point) >> (px > 0 && py > 0), _ >> false));
}

TEST(ExpressionHandler, RangePredicatesUseBoundValue) {
  int n = 4;
  EXPECT_TRUE(match(n) | on($ >> rng(1, 5)));
  n = 6;
  EXPECT_FALSE(match(n) | on($ >> rng(1, 5)));
}

TEST(ExpressionHandler, VariantBindingAndFallback) {
  std::variant<int, std::string> value = 3;
  auto cases = on($(is<int>) >> _ * 2 + 1, _ >> -1);
  EXPECT_EQ(match(value) | cases, 7);
  value = std::string("other");
  EXPECT_EQ(match(value) | cases, -1);
}

TEST(ExpressionHandler, TypeSubpatternResolvesMembers) {
  std::variant<int, Point> value = Point{3, 4};
  EXPECT_EQ(
      match(value) | on(is<Point>($(point)) >> px + py, _ >> 0), 7);
  EXPECT_EQ(match(value) | on(alt<1>($(point)) >> px - py, _ >> 0),
            -1);
}

TEST(ExpressionHandler, OnlySelectedCaseEvaluatesHandler) {
  int n = 0;
  EXPECT_EQ(match(n) | on(lit(0) >> 42, $ >> 10 / _), 42);
  EXPECT_EQ(match(n) | on($[_ != 0] >> 10 / _, $ >> _ + 1), 1);
}

TEST(ExpressionHandler, FullyQualifiedAndStaticCases) {
  int n = 3;
  EXPECT_EQ(ptn::match(n) | ptn::on(ptn::$ >> ptn::_ * 2), 6);
  EXPECT_EQ(match(n) | PTN_ON($ >> _ + 1), 4);
  Point p{3, 4};
  EXPECT_EQ(match(p) | PTN_ON($(point) >> px + py, _ >> 0), 7);
}

TEST(ExpressionHandler, PreservesCommonReturnType) {
  int  n      = 3;
  auto cases  = on(lit(0) >> 0.5, $ >> _ * 2);
  auto result = match(n) | cases;
  static_assert(std::is_same_v<decltype(result), double>);
  EXPECT_DOUBLE_EQ(result, 6.0);
}
