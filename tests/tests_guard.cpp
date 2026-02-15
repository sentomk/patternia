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
                          .when(ptn::bind()[ptn::_ > 0 && ptn::_ < 10] >> 1)
                          .otherwise(0);

  int outside_result = ptn::match(outside)
                           .when(ptn::bind()[ptn::_ > 0 && ptn::_ < 10] >> 1)
                           .otherwise(0);

  EXPECT_EQ(inside_result, 1);
  EXPECT_EQ(outside_result, 0);
}

TEST(Guard, RangeHelperModes) {
  int boundary = 10;

  int closed_result =
      ptn::match(boundary).when(ptn::bind()[ptn::rng(0, 10)] >> 1).otherwise(0);

  int open_result = ptn::match(boundary)
                        .when(
                            ptn::bind()[ptn::rng(0, 10, ptn::pat::mod::open)]
                            >> 1)
                        .otherwise(0);

  EXPECT_EQ(closed_result, 1);
  EXPECT_EQ(open_result, 0);
}

TEST(Guard, MultiArgExpressionPredicate) {
  Point p{3, 4};

  int result =
      ptn::match(p)
          .when(
              ptn::bind(ptn::has<&Point::x, &Point::y>())
                  [ptn::arg<0> * ptn::arg<0> + ptn::arg<1> * ptn::arg<1> == 25]
              >> 1)
          .otherwise(0);

  EXPECT_EQ(result, 1);
}

TEST(Guard, MultiArgCallablePredicate) {
  Point p{2, 5};

  int result = ptn::match(p)
                   .when(ptn::bind(ptn::has<&Point::x, &Point::y>())
                             [([](int x, int y) { return x < y; })]
                         >> 1)
                   .otherwise(0);

  EXPECT_EQ(result, 1);
}
