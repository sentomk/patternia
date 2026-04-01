#include <gtest/gtest.h>

#include <ptn/patternia.hpp>

#include <tuple>

using namespace ptn;

struct ProbePattern : ptn::pat::base::pattern_base<ProbePattern> {
  bool should_match;
  int *calls;

  template <typename Subject>
  constexpr bool match(const Subject &) const noexcept {
    ++(*calls);
    return should_match;
  }

  template <typename Subject>
  constexpr auto bind(const Subject &) const {
    return std::tuple<>{};
  }
};

namespace ptn::pat::base {

  template <typename Subject>
  struct binding_args<::ProbePattern, Subject> {
    using type = std::tuple<>;
  };

} // namespace ptn::pat::base

TEST(CombinatorPattern, AnyShortCircuitsAfterFirstHit) {
  int c1 = 0;
  int c2 = 0;
  int c3 = 0;

  int x = 7;
  int result = match(x) | on(any(ProbePattern{false, &c1},
                                 ProbePattern{true, &c2},
                                 ProbePattern{true, &c3})
                                 >> 1,
                             __ >> 0);

  EXPECT_EQ(result, 1);
  EXPECT_EQ(c1, 1);
  EXPECT_EQ(c2, 1);
  EXPECT_EQ(c3, 0);
}

TEST(CombinatorPattern, AnyFallsBackWhenAllMiss) {
  int c1 = 0;
  int c2 = 0;

  int x = 7;
  int result = match(x) | on(any(ProbePattern{false, &c1},
                                 ProbePattern{false, &c2})
                                 >> 1,
                             __ >> 0);

  EXPECT_EQ(result, 0);
  EXPECT_EQ(c1, 1);
  EXPECT_EQ(c2, 1);
}

TEST(CombinatorPattern, AllMatchesOnlyWhenAllHit) {
  int c1 = 0;
  int c2 = 0;
  int c3 = 0;

  int x = 7;
  int result = match(x) | on(all(ProbePattern{true, &c1},
                                 ProbePattern{true, &c2},
                                 ProbePattern{true, &c3})
                                 >> 1,
                             __ >> 0);

  EXPECT_EQ(result, 1);
  EXPECT_EQ(c1, 1);
  EXPECT_EQ(c2, 1);
  EXPECT_EQ(c3, 1);
}

TEST(CombinatorPattern, AllShortCircuitsOnFirstMiss) {
  int c1 = 0;
  int c2 = 0;
  int c3 = 0;

  int x = 7;
  int result = match(x) | on(all(ProbePattern{true, &c1},
                                 ProbePattern{false, &c2},
                                 ProbePattern{true, &c3})
                                 >> 1,
                             __ >> 0);

  EXPECT_EQ(result, 0);
  EXPECT_EQ(c1, 1);
  EXPECT_EQ(c2, 1);
  EXPECT_EQ(c3, 0);
}

TEST(CombinatorPattern, AnyAndAllWorkWithZeroBindHandlers) {
  int x = 2;

  int any_result = match(x) | on(any(lit(1), lit(2)) >> [] { return 11; },
                                 __ >> 0);

  int all_result = match(x) | on(all(any(lit(2), lit(3)), lit(2))
                                     >> [] { return 22; },
                                 __ >> 0);

  EXPECT_EQ(any_result, 11);
  EXPECT_EQ(all_result, 22);
}
