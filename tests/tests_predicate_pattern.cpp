#include <gtest/gtest.h>
#include "ptn/patternia.hpp"

using namespace ptn;

TEST(PredicatePattern, BasicLambda) {
  auto is_even = pred([](int x) { return x % 2 == 0; });
  EXPECT_TRUE(is_even(2));
  EXPECT_FALSE(is_even(3));
}

TEST(PredicatePattern, LogicalCompose) {
  auto is_even     = pred([](int x) { return x % 2 == 0; });
  auto is_pos      = pred([](int x) { return x > 0; });
  auto is_even_pos = is_even && is_pos;
  EXPECT_TRUE(is_even_pos(2));
  EXPECT_FALSE(is_even_pos(-2));
  EXPECT_FALSE(is_even_pos(3));
}

TEST(PredicatePattern, LogicalNotOr) {
  auto is_even = pred([](int x) { return x % 2 == 0; });
  auto is_odd  = !is_even;
  EXPECT_TRUE(is_odd(3));
  EXPECT_FALSE(is_odd(4));

  auto is_neg         = pred([](int x) { return x < 0; });
  auto is_even_or_neg = is_even || is_neg;
  EXPECT_TRUE(is_even_or_neg(-1));
  EXPECT_TRUE(is_even_or_neg(4));
  EXPECT_FALSE(is_even_or_neg(3));
}

TEST(PredicatePattern, CapturedLambda) {
  int  threshold = 10;
  auto is_large  = pred([threshold](int x) { return x > threshold; });
  EXPECT_TRUE(is_large(11));
  EXPECT_FALSE(is_large(5));
}

TEST(PredicatePattern, WithMatch) {
  auto result =
      match(5)
          .when(pred([](int v) { return v % 2 == 0; }) >> [] { return "even"; })
          .when(pred([](int v) { return v % 2 == 1; }) >> [] { return "odd"; })
          .otherwise([] { return "none"; });

  EXPECT_EQ(result, "odd");
}
