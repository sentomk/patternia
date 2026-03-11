#include <gtest/gtest.h>

#include <ptn/patternia.hpp>

using namespace ptn;

namespace {

  struct Point {
    int x;
    int y;
  };

} // namespace

TEST(Guard, UnaryPlaceholderPredicate) {
  int inside  = 6;
  int outside = 20;

  int inside_result = ptn::match(inside)
                      | ptn::on(ptn::bind()[ptn::_0 > 0 && ptn::_0 < 10] >> 1,
                                ptn::__ >> 0);

  int outside_result = ptn::match(outside)
                       | ptn::on(
                           ptn::bind()[ptn::_0 > 0 && ptn::_0 < 10] >> 1,
                           ptn::__ >> 0);

  EXPECT_EQ(inside_result, 1);
  EXPECT_EQ(outside_result, 0);
}

TEST(Guard, RangeHelperModes) {
  int boundary = 10;

  int closed_result = ptn::match(boundary)
                      | ptn::on(ptn::bind()[ptn::rng(0, 10)] >> 1,
                                ptn::__ >> 0);

  int open_result = ptn::match(boundary)
                    | ptn::on(
                        ptn::bind()[ptn::rng(0, 10, ptn::pat::mod::open)] >> 1,
                        ptn::__ >> 0);

  EXPECT_EQ(closed_result, 1);
  EXPECT_EQ(open_result, 0);
}

TEST(Guard, MultiArgExpressionPredicate) {
  Point p{3, 4};

  int result = ptn::match(p)
               | ptn::on(ptn::bind(ptn::has<&Point::x, &Point::y>())
                             [ptn::arg<0> * ptn::arg<0>
                                  + ptn::arg<1> * ptn::arg<1>
                              == 25]
                             >> 1,
                         ptn::__ >> 0);

  EXPECT_EQ(result, 1);
}

TEST(Guard, MultiArgCallablePredicate) {
  Point p{2, 5};

  int result = ptn::match(p)
               | ptn::on(ptn::bind(ptn::has<&Point::x, &Point::y>())[(
                             [](int x, int y) { return x < y; })]
                             >> 1,
                         ptn::__ >> 0);

  EXPECT_EQ(result, 1);
}

// -- _N placeholder aliases --

TEST(Guard, ZeroPlaceholderSingleValue) {
  int x      = 7;
  int result = match(x) | on($[_0 > 5] >> [](int v) { return v; }, _ >> 0);

  EXPECT_EQ(result, 7);
}

TEST(Guard, ZeroPlaceholderCompoundGuard) {
  int x      = 6;
  int result =
      match(x) | on($[_0 > 0 && _0 < 10] >> [](int v) { return v; }, _ >> -1);

  EXPECT_EQ(result, 6);
}

TEST(Guard, MultiArgWithPlaceholderAliases) {
  Point p{3, 4};

  int result = match(p)
               | on(bind(has<&Point::x, &Point::y>())[_0 * _0 + _1 * _1 == 25]
                        >> 1,
                    _ >> 0);

  EXPECT_EQ(result, 1);
}

TEST(Guard, PlaceholderAliasEquivalentToArg) {
  Point p{2, 5};

  int r1 = match(p)
           | on(bind(has<&Point::x, &Point::y>())[_0 + _1 == 7] >> 1, _ >> 0);

  int r2 = match(p)
           | on(bind(has<&Point::x, &Point::y>())[arg<0> + arg<1> == 7] >> 1,
                _ >> 0);

  EXPECT_EQ(r1, r2);
}

TEST(Guard, MixedGuardExprAndCallable) {
  int  x      = 8;
  auto even   = [](auto v) { return v % 2 == 0; };
  int  result = match(x) | on($[_0 > 5 && even] >> [](int v) { return v; }, _ >> -1);

  EXPECT_EQ(result, 8);
}

TEST(Guard, MixedGuardCallableRejects) {
  int  x      = 7;
  auto even   = [](auto v) { return v % 2 == 0; };
  int  result = match(x) | on($[_0 > 5 && even] >> [](int v) { return v; }, _ >> -1);

  EXPECT_EQ(result, -1);
}
