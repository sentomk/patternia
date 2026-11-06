// Includes Patternia before any other header, so the umbrella header
// has to bring in every standard header it uses itself.
#include <ptn/patternia.hpp>

#include <gtest/gtest.h>

TEST(HeaderFirst, PatterniaCompilesWithoutPriorIncludes) {
  int x = 1;

  EXPECT_EQ(ptn::match(x) | ptn::on(ptn::lit(1) >> 0, ptn::_ >> 1),
            0);
}
