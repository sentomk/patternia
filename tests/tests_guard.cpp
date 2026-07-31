#include <gtest/gtest.h>

#include <ptn/patternia.hpp>

using namespace ptn;

namespace {

  struct Point {
    int x;
    int y;
  };

  PTN_BIND(Point, x, y);

} // namespace

TEST(Guard, WildcardActsAsSingleValuePlaceholder) {
  int inside  = 6;
  int outside = 20;

  int inside_result = match(inside)
                      | on($[_ > 0 && _ < 10] >> 1, _ >> 0);
  int outside_result = match(outside)
                       | on($[_ > 0 && _ < 10] >> 1, _ >> 0);

  EXPECT_EQ(inside_result, 1);
  EXPECT_EQ(outside_result, 0);
}

TEST(Guard, WildcardPlaceholderSupportsArithmetic) {
  int value  = 5;
  int result = match(value) | on($[_ * _ == 25] >> 1, _ >> 0);

  EXPECT_EQ(result, 1);
}

TEST(Guard, WildcardPlaceholderSupportsAllBinaryOperators) {
  int value  = 5;
  int result = match(value)
               | on($[(_ + 1 == 6) && (_ - 1 == 4) && (_ * 2 == 10)
                      && (_ / 5 == 1) && (_ % 2 == 1) && (_ != 4)
                      && (_ < 6) && (_ <= 5) && (_ > 4) && (_ >= 5)
                      && (4 < _)]
                        >> 1,
                    $[_ == 0 || _ == 5] >> 2,
                    _ >> 0);

  EXPECT_EQ(result, 1);
}

TEST(Guard, WildcardPatternKeepsItsValueTypeProperties) {
  using wildcard_t = std::decay_t<decltype(ptn::_)>;

  static_assert(std::is_empty_v<wildcard_t>);
  static_assert(std::is_trivially_copyable_v<wildcard_t>);
  static_assert(sizeof(wildcard_t) == 1);
}

TEST(Guard, RangeHelperModes) {
  int boundary = 10;

  int closed_result = ptn::match(boundary)
                      | ptn::on(ptn::$[ptn::rng(0, 10)] >> 1,
                                ptn::_ >> 0);

  int open_result = ptn::match(boundary)
                    | ptn::on(
                        ptn::$[ptn::rng(0, 10, ptn::pat::mod::open)]
                            >> 1,
                        ptn::_ >> 0);

  EXPECT_EQ(closed_result, 1);
  EXPECT_EQ(open_result, 0);
}

TEST(Guard, NamedMultiValueExpressionPredicate) {
  Point point{3, 4};

  int result = match(point)
               | on($(has<&Point::x, &Point::y>)[x * x + y * y == 25]
                        >> 1,
                    _ >> 0);

  EXPECT_EQ(result, 1);
}

TEST(Guard, MultiValueCallablePredicate) {
  Point point{2, 5};

  int result = match(point)
               | on($(has<&Point::x, &Point::y>)[([](int left,
                                                     int right) {
                      return left < right;
                    })] >> 1,
                    _ >> 0);

  EXPECT_EQ(result, 1);
}

TEST(Guard, WildcardPlaceholderComposesWithCallablePredicate) {
  int  value  = 8;
  auto even   = [](auto current) { return current % 2 == 0; };
  int  result = match(value)
               | on(
                   $[_ > 5 && even] >>
                       [](int current) { return current; },
                   _ >> -1);

  EXPECT_EQ(result, 8);
}

TEST(Guard, WildcardPlaceholderCallableCanReject) {
  int  value  = 7;
  auto even   = [](auto current) { return current % 2 == 0; };
  int  result = match(value)
               | on(
                   $[_ > 5 && even] >>
                       [](int current) { return current; },
                   _ >> -1);

  EXPECT_EQ(result, -1);
}

TEST(Guard, BlockScopeNamesSupportFiveValues) {
  struct Record {
    int a;
    int b;
    int c;
    int d;
    int e;
  };

  PTN_BIND(Record, a, b, c, d, e);

  Record record{1, 2, 3, 4, 10};
  int    result = match(record)
               | on($(has<&Record::a,
                          &Record::b,
                          &Record::c,
                          &Record::d,
                          &Record::e>)[a + b + c + d == e]
                        >> 1,
                    _ >> 0);

  EXPECT_EQ(result, 1);
}
