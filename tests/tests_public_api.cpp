#include <gtest/gtest.h>

#include <ptn/patternia.hpp>

TEST(PublicApi, FullyQualifiedPipelineOnWorksWithoutUsingDirective) {
  int x = 1;

  int result = ptn::match(x)
               | ptn::on(
                   ptn::lit(1) >> 42,
                   ptn::__ >> 0
               );

  EXPECT_EQ(result, 42);
}

TEST(PublicApi, FullyQualifiedPipelinePtnOnWorksWithoutUsingDirective) {
  int x = 2;

  int result = ptn::match(x)
               | PTN_ON(
                   ptn::val<1> >> 1,
                   ptn::val<2> >> 2,
                   ptn::__ >> 0
               );

  EXPECT_EQ(result, 2);
}
