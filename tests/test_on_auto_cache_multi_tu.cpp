// Per C++17 [basic.def.odr]/6 + [basic.link]/8, function-local
// statics inside an inline function template (or vague-linkage
// member function template) with identical template args are
// ODR-merged into ONE program-wide object. This test asserts that
// contract — and exists to PROTECT it from regressions (e.g. someone
// adding __attribute__((noinline)) or per-TU specialization). Named
// empty-handler structs from the shared header (NOT inline lambdas)
// guarantee type-identity of Cases... across TUs.

#include "auto_cache_shared_handlers.hpp"
#include "ptn/patternia.hpp"
#include <gtest/gtest.h>

using namespace ptn;
using namespace auto_cache_shared;

extern "C" int     tu_a_match(int);
extern const void *tu_a_last_addr;

namespace {

  TEST(OnAutoCacheMultiTU, OdrMergedSlotsAcrossTU) {
#ifdef PTN_TESTING
    // Call helper TU (tu_a_match in
    // test_on_auto_cache_multi_tu_helper.cpp)
    EXPECT_EQ(tu_a_match(3), 6);
    const void *tu_a_addr = tu_a_last_addr;
    EXPECT_NE(tu_a_addr, nullptr) << "TU-A sentinel not written";

    // Call this TU directly — identical Cases... types via shared
    // header structs
    int v      = 5;
    int result = match(v) | on($ >> DoubleIt{}, _ >> ReturnZero{});
    EXPECT_EQ(result, 10);
    const void *tu_b_addr = ptn::core::engine::detail::
        ptn_testing_last_cached_addr;
    EXPECT_NE(tu_b_addr, nullptr) << "TU-B sentinel not written";

    // ODR-merged single program-wide slot — addresses MUST be equal
    EXPECT_EQ(tu_a_addr, tu_b_addr)
        << "Cache slots differ across TUs — ODR merging may have "
           "broken";
#else
    GTEST_SKIP() << "PTN_TESTING not enabled";
#endif
  }

} // anonymous namespace
