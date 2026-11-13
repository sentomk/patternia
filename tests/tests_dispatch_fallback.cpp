#include <gtest/gtest.h>

#include <ptn/patternia.hpp>

#include <string>
#include <tuple>
#include <variant>

using namespace ptn;

namespace {
  using ptn::core::common::eval_cases_typed;

  // The evaluator also supports incomplete case tuples. Public on()
  // requires a terminal case, so test its otherwise contract here.
  template <typename Cases>
  void expect_integer_fallback(Cases cases) {
    int  calls     = 0;
    auto otherwise = [&](int &value) -> short {
      ++calls;
      return static_cast<short>(-value);
    };
    for (int value : {-1, 0, 1, 2, 3, 4}) {
      const int  before = calls;
      const bool hit    = value == 0 || value == 2;
      EXPECT_EQ(eval_cases_typed<long>(value, cases, otherwise),
                hit ? value + 10 : -value);
      EXPECT_EQ(calls - before, hit ? 0 : 1);
    }
  }
} // namespace

TEST(DispatchFallback, StaticLiteralsFallBackInsideAndOutsideTable) {
  expect_integer_fallback(
      std::make_tuple(val<0> >> 10, val<2> >> 12));
}

TEST(DispatchFallback, RuntimeLiteralsFallBackOnce) {
  expect_integer_fallback(
      std::make_tuple(lit(0) >> 10, lit(2) >> 12));
}

TEST(DispatchFallback, PredicatesFallBackOnce) {
  expect_integer_fallback(std::make_tuple(
      pred([](int value) { return value == 0; }) >> 10,
      pred([](int value) { return value == 2; }) >> 12));
}

TEST(DispatchFallback, EmptyCasesAcceptNullaryAndSubjectFallbacks) {
  auto cases   = std::tuple<>{};
  int  subject = 9;
  int  calls   = 0;
  EXPECT_EQ(eval_cases_typed<long>(subject,
                                   cases,
                                   [&]() -> short {
                                     ++calls;
                                     return 4;
                                   }),
            4);
  EXPECT_EQ(calls, 1);
  eval_cases_typed<void>(subject, cases, [&](int &value) {
    ++calls;
    value = 12;
  });
  EXPECT_EQ(calls, 2);
  EXPECT_EQ(subject, 12);
  eval_cases_typed<void>(subject, cases, [&] { ++calls; });
  EXPECT_EQ(calls, 3);
}

TEST(DispatchFallback, VariantWithoutMatchingAlternativeFallsBack) {
  using V        = std::variant<int, std::string, double>;
  auto cases     = std::make_tuple(is<int> >> 7);
  int  calls     = 0;
  auto otherwise = [&](const V &v) {
    ++calls;
    return -static_cast<int>(v.index());
  };
  V subject = 1;
  EXPECT_EQ(eval_cases_typed<int>(subject, cases, otherwise), 7);
  EXPECT_EQ(calls, 0);
  subject = std::string("missing");
  EXPECT_EQ(eval_cases_typed<int>(subject, cases, otherwise), -1);
  subject = 2.5;
  EXPECT_EQ(eval_cases_typed<int>(subject, cases, otherwise), -2);
  EXPECT_EQ(calls, 2);
}

TEST(DispatchFallback, VariantGuardRejectionFallsBackOnce) {
  using V        = std::variant<int, std::string>;
  auto cases     = std::make_tuple($(is<int>)[_ > 0] >> (_ + 1));
  int  calls     = 0;
  auto otherwise = [&] {
    ++calls;
    return -1;
  };
  V subject = 3;
  EXPECT_EQ(eval_cases_typed<int>(subject, cases, otherwise), 4);
  EXPECT_EQ(calls, 0);
  subject = -2;
  EXPECT_EQ(eval_cases_typed<int>(subject, cases, otherwise), -1);
  subject = std::string("missing");
  EXPECT_EQ(eval_cases_typed<int>(subject, cases, otherwise), -1);
  EXPECT_EQ(calls, 2);
}

namespace {
  template <std::size_t... I>
  void expect_variant_fallbacks(std::index_sequence<I...>) {
    using V = std::variant<
        std::integral_constant<std::size_t, I>...>;
    constexpr std::size_t last = sizeof...(I) - 1;
    auto cases     = std::make_tuple(alt<0> >> 1, alt<last> >> 2);
    int  calls     = 0;
    auto otherwise = [&](const V &v) {
      ++calls;
      return -static_cast<int>(v.index());
    };
    auto check = [&](auto index) {
      constexpr std::size_t i = decltype(index)::value;
      V                     subject(std::in_place_index<i>);
      int                   before = calls;
      EXPECT_EQ(eval_cases_typed<int>(subject, cases, otherwise),
                i == 0 ? 1 : (i == last ? 2 : -static_cast<int>(i)));
      EXPECT_EQ(calls - before, i == 0 || i == last ? 0 : 1);
    };
    (check(std::integral_constant<std::size_t, I>{}), ...);
  }
} // namespace

TEST(DispatchFallback, VariantInlineTierPreservesFallbacks) {
  expect_variant_fallbacks(std::make_index_sequence<8>{});
}

TEST(DispatchFallback, VariantSegmentedTierPreservesFallbacks) {
  expect_variant_fallbacks(std::make_index_sequence<16>{});
}

TEST(DispatchFallback, VariantCompactTierPreservesFallbacks) {
  expect_variant_fallbacks(std::make_index_sequence<65>{});
}
