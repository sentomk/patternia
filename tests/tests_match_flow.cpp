#include <gtest/gtest.h>

#include <ptn/patternia.hpp>

#include <tuple>

using namespace ptn;

struct ForwardingProbePattern
    : ptn::pat::base::pattern_base<ForwardingProbePattern>,
      ptn::pat::base::binding_pattern_base<ForwardingProbePattern> {
  static int lvalue_bind_calls;
  static int rvalue_bind_calls;

  template <typename Subject>
  constexpr bool match(const Subject &) const noexcept {
    return true;
  }

  auto bind(const int &subject) const {
    ++lvalue_bind_calls;
    return std::tuple<int>{subject};
  }

  auto bind(int &&subject) const {
    ++rvalue_bind_calls;
    return std::tuple<int>{subject};
  }
};

int ForwardingProbePattern::lvalue_bind_calls = 0;
int ForwardingProbePattern::rvalue_bind_calls = 0;

struct ConditionalProbePattern
    : ptn::pat::base::pattern_base<ConditionalProbePattern>,
      ptn::pat::base::binding_pattern_base<ConditionalProbePattern> {
  static bool should_match;
  static int  bind_calls;

  template <typename Subject>
  constexpr bool match(const Subject &) const noexcept {
    return should_match;
  }

  auto bind(const int &subject) const {
    ++bind_calls;
    return std::tuple<int>{subject};
  }
};

bool ConditionalProbePattern::should_match = false;
int  ConditionalProbePattern::bind_calls   = 0;

namespace ptn::pat::base {

  template <typename Subject>
  struct binding_args<::ForwardingProbePattern, Subject> {
    using type = std::tuple<int>;
  };

  template <typename Subject>
  struct binding_args<::ConditionalProbePattern, Subject> {
    using type = std::tuple<int>;
  };

} // namespace ptn::pat::base

TEST(MatchFlow, OtherwiseCallableWithSubject) {
  int x      = 7;
  int result = ptn::match(x).when(ptn::lit(1) >> 10).otherwise([](int v) {
    return v * 2;
  });

  EXPECT_EQ(result, 14);
}

TEST(MatchFlow, WildcardEndFlow) {
  int x   = 9;
  int hit = 0;

  ptn::match(x)
      .when(ptn::lit(1) >> [&] { hit = 1; })
      .when(ptn::__ >> [&] { hit = 2; })
      .end();

  EXPECT_EQ(hit, 2);
}

TEST(MatchFlow, SubjectBindsAsLvalue) {
  ForwardingProbePattern::lvalue_bind_calls = 0;
  ForwardingProbePattern::rvalue_bind_calls = 0;

  int x      = 11;
  int result = ptn::match(x)
                   .when(ForwardingProbePattern{} >> [](int v) { return v; })
                   .otherwise(-1);

  EXPECT_EQ(result, 11);
  EXPECT_EQ(ForwardingProbePattern::lvalue_bind_calls, 1);
  EXPECT_EQ(ForwardingProbePattern::rvalue_bind_calls, 0);
}

TEST(MatchFlow, GuardedCaseBindsOnlyOnceOnMatch) {
  ForwardingProbePattern::lvalue_bind_calls = 0;
  ForwardingProbePattern::rvalue_bind_calls = 0;

  int x      = 11;
  int result = ptn::match(x)
                   .when(ForwardingProbePattern{}[ptn::_ > 0] >>
                         [](int v) { return v; })
                   .otherwise(-1);

  EXPECT_EQ(result, 11);
  EXPECT_EQ(ForwardingProbePattern::lvalue_bind_calls, 1);
  EXPECT_EQ(ForwardingProbePattern::rvalue_bind_calls, 0);
}

TEST(MatchFlow, GuardedPathOneBindVsLegacyTwoBindSequence) {
  int x = 11;

  auto guarded = ForwardingProbePattern{}[ptn::_ > 0];

  // Simulate legacy two-step flow:
  // 1) guarded.match(x) -> inner.bind(x) inside guard evaluation
  // 2) guarded.bind(x)  -> bind again before handler invocation
  ForwardingProbePattern::lvalue_bind_calls = 0;
  ForwardingProbePattern::rvalue_bind_calls = 0;

  if (guarded.match(x)) {
    auto bound = guarded.bind(x);
    (void)std::get<0>(bound);
  }

  EXPECT_EQ(ForwardingProbePattern::lvalue_bind_calls, 2);
  EXPECT_EQ(ForwardingProbePattern::rvalue_bind_calls, 0);

  // Current optimized engine path should bind only once on guard-hit.
  ForwardingProbePattern::lvalue_bind_calls = 0;
  ForwardingProbePattern::rvalue_bind_calls = 0;

  int result = ptn::match(x)
                   .when(ForwardingProbePattern{}[ptn::_ > 0] >>
                         [](int v) { return v; })
                   .otherwise(-1);

  EXPECT_EQ(result, 11);
  EXPECT_EQ(ForwardingProbePattern::lvalue_bind_calls, 1);
  EXPECT_EQ(ForwardingProbePattern::rvalue_bind_calls, 0);
}

TEST(MatchFlow, BindCountMatrixNoBindWhenPatternMisses) {
  ConditionalProbePattern::should_match = false;
  ConditionalProbePattern::bind_calls   = 0;

  int x      = 11;
  int result = ptn::match(x)
                   .when(ConditionalProbePattern{} >> [](int) { return 1; })
                   .otherwise(0);

  EXPECT_EQ(result, 0);
  EXPECT_EQ(ConditionalProbePattern::bind_calls, 0);
}

TEST(MatchFlow, BindCountMatrixOneBindWhenPatternMatches) {
  ConditionalProbePattern::should_match = true;
  ConditionalProbePattern::bind_calls   = 0;

  int x      = 11;
  int result = ptn::match(x)
                   .when(ConditionalProbePattern{} >> [](int v) { return v; })
                   .otherwise(-1);

  EXPECT_EQ(result, 11);
  EXPECT_EQ(ConditionalProbePattern::bind_calls, 1);
}

TEST(MatchFlow, BindCountMatrixGuardMissBindsOnceThenOtherwise) {
  ForwardingProbePattern::lvalue_bind_calls = 0;
  ForwardingProbePattern::rvalue_bind_calls = 0;

  int x      = 11;
  int result = ptn::match(x)
                   .when(ForwardingProbePattern{}[ptn::_ > 100] >>
                         [](int) { return 1; })
                   .otherwise(0);

  EXPECT_EQ(result, 0);
  EXPECT_EQ(ForwardingProbePattern::lvalue_bind_calls, 1);
  EXPECT_EQ(ForwardingProbePattern::rvalue_bind_calls, 0);
}

TEST(MatchFlow, BindCountMatrixGuardMissThenNextCaseAddsSecondBind) {
  ForwardingProbePattern::lvalue_bind_calls = 0;
  ForwardingProbePattern::rvalue_bind_calls = 0;

  int x      = 11;
  int result = ptn::match(x)
                   .when(ForwardingProbePattern{}[ptn::_ > 100] >>
                         [](int) { return -1; })
                   .when(ForwardingProbePattern{} >> [](int v) { return v; })
                   .otherwise(0);

  EXPECT_EQ(result, 11);
  EXPECT_EQ(ForwardingProbePattern::lvalue_bind_calls, 2);
  EXPECT_EQ(ForwardingProbePattern::rvalue_bind_calls, 0);
}
