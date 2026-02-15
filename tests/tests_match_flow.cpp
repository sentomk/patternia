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

namespace ptn::pat::base {

  template <typename Subject>
  struct binding_args<::ForwardingProbePattern, Subject> {
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
