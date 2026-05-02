#include "ptn/patternia.hpp"
#include <gtest/gtest.h>

namespace {

  TEST(OnAutoCacheIdentity, PositiveStatelessPackCached) {
#ifdef PTN_TESTING
    using namespace ptn;

    auto run = [](int x) {
      return match(x)
             | on(
                 val<1> >> [] { return 10; },
                 val<2> >> [] { return 20; },
                 _ >> [] { return 0; });
    };

    EXPECT_EQ(run(1), 10);
    const void *first = ptn::core::engine::detail::
        ptn_testing_last_cached_addr;
    EXPECT_NE(first, nullptr) << "Sentinel not written";

    EXPECT_EQ(run(2), 20);
    const void *second = ptn::core::engine::detail::
        ptn_testing_last_cached_addr;
    EXPECT_EQ(first, second) << "Cache address changed";

    for (int i = 0; i < 10000; ++i) {
      EXPECT_EQ(run(i % 3), (i % 3 == 1 ? 10 : i % 3 == 2 ? 20 : 0));
      EXPECT_EQ(
          ptn::core::engine::detail::ptn_testing_last_cached_addr,
          first)
          << "Address changed at iter " << i;
    }
#else
    GTEST_SKIP() << "PTN_TESTING not enabled";
#endif
  }

  TEST(OnAutoCacheIdentity, NegativeStatefulPackNotCached) {
#ifdef PTN_TESTING
    using namespace ptn;
    const void *before = ptn::core::engine::detail::
        ptn_testing_last_cached_addr;

    int  v1     = 1;
    auto result = match(v1) | on(val<1> >> 1, _ >> 0);
    EXPECT_EQ(result, 1);

    const void *after = ptn::core::engine::detail::
        ptn_testing_last_cached_addr;
    EXPECT_EQ(before, after) << "Stateful pack unexpectedly cached";
#else
    GTEST_SKIP() << "PTN_TESTING not enabled";
#endif
  }

  TEST(OnAutoCacheIdentity, MixedStatelessAndStateful) {
#ifdef PTN_TESTING
    using namespace ptn;

    auto run_stateless = [](int x) {
      return match(x)
             | on(
                 val<1> >> [] { return 10; }, _ >> [] { return 0; });
    };

    const void *before = ptn::core::engine::detail::
        ptn_testing_last_cached_addr;
    int x1 = 1;
    EXPECT_EQ(run_stateless(x1), 10);
    const void *after_stateless = ptn::core::engine::detail::
        ptn_testing_last_cached_addr;
    EXPECT_NE(after_stateless, before);

    int x2 = 2;
    EXPECT_EQ(match(x2) | on(val<2> >> 2, _ >> 0), 2);
    const void *after_stateful = ptn::core::engine::detail::
        ptn_testing_last_cached_addr;
    EXPECT_EQ(after_stateful, after_stateless)
        << "Stateful touched cache sentinel";

    EXPECT_EQ(run_stateless(x1), 10);
    EXPECT_EQ(
        ptn::core::engine::detail::ptn_testing_last_cached_addr,
        after_stateless)
        << "Second stateless call got different address";
#else
    GTEST_SKIP() << "PTN_TESTING not enabled";
#endif
  }

} // namespace
