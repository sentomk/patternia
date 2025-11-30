#include <gtest/gtest.h>
#include "ptn/patternia.hpp"

using namespace ptn;
using namespace ptn::pat::value;
using namespace ptn::core::dsl::ops;

TEST(PredicatePattern, LogicalCompose) {
  auto is_even     = pred([](int x) { return x % 2 == 0; });
  auto is_pos      = pred([](int x) { return x > 0; });
  auto is_even_pos = is_even && is_pos;
  EXPECT_TRUE(is_even_pos.match(2));
  EXPECT_FALSE(is_even_pos.match(-2));
  EXPECT_FALSE(is_even_pos.match(3));
}
