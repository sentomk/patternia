#include <gtest/gtest.h>

#include <ptn/patternia.hpp>

#include <array>
#include <cstdint>
#include <limits>
#include <string>
#include <tuple>
#include <variant>

using namespace ptn;
namespace detail = ptn::core::common::detail;

TEST(DispatchMetadata,
     CompactMapPreservesOrderAndEmptyAlternatives) {
  using bucket           = detail::variant_typed_case_bucket;
  using map_type         = std::array<std::uint8_t, 4>;
  constexpr auto invalid = std::numeric_limits<std::uint8_t>::max();
  struct Scenario {
    std::array<bucket, 4> buckets;
    map_type              expected;
    std::size_t           count;
  };
  const Scenario scenarios[] = {
      {{bucket{4, 0, false},
        bucket{4, 0, false},
        bucket{4, 0, false},
        bucket{4, 0, false}},
       {invalid, invalid, invalid, invalid},
       0},
      {{bucket{0, 2, true},
        bucket{4, 0, false},
        bucket{2, 4, true},
        bucket{4, 0, false}},
       {0, invalid, 1, invalid},
       2},
      {{bucket{4, 0, false},
        bucket{1, 3, true},
        bucket{4, 0, false},
        bucket{3, 4, true}},
       {invalid, 0, invalid, 1},
       2},
      {{bucket{0, 1, true},
        bucket{1, 2, true},
        bucket{2, 3, true},
        bucket{3, 4, true}},
       {0, 1, 2, 3},
       4},
  };
  for (const auto &scenario : scenarios) {
    const auto indexes = std::make_index_sequence<4>{};
    EXPECT_EQ(detail::count_typed_variant_compact_dispatch_alts(
                  scenario.buckets, indexes),
              scenario.count);
    EXPECT_EQ((detail::make_typed_variant_compact_alt_index_map<
                  std::uint8_t,
                  4>(scenario.buckets, indexes)),
              scenario.expected);
  }
}

TEST(DispatchMetadata, LiteralWidthsIncludeBothEndpoints) {
  struct SignedRange {
    std::int32_t lo, hi;
    std::size_t  width;
  };
  const auto        min = std::numeric_limits<std::int32_t>::min();
  const auto        max = std::numeric_limits<std::int32_t>::max();
  const SignedRange signed_ranges[] = {{0, 0, 1},
                                       {-6, -2, 5},
                                       {-128, 127, 256},
                                       {min, min + 3, 4},
                                       {max - 3, max, 4}};
  for (const auto &range : signed_ranges) {
    EXPECT_EQ(detail::static_literal_range_width(range.lo, range.hi),
              range.width);
  }
  struct UnsignedRange {
    std::uint32_t lo, hi;
    std::size_t   width;
  };
  const auto
      unsigned_max = std::numeric_limits<std::uint32_t>::max();
  const UnsignedRange unsigned_ranges[] = {
      {0, 0, 1}, {0, 255, 256}, {unsigned_max - 3, unsigned_max, 4}};
  for (const auto &range : unsigned_ranges) {
    EXPECT_EQ(detail::static_literal_range_width(range.lo, range.hi),
              range.width);
  }
}

namespace {
  void expect_residual_slices(
      const std::array<detail::variant_typed_case_bucket, 4>
          &buckets) {
    EXPECT_EQ(buckets[0].begin, 0u);
    EXPECT_EQ(buckets[0].end, 4u);
    EXPECT_TRUE(buckets[0].has_any);
    EXPECT_EQ(buckets[1].begin, 2u);
    EXPECT_EQ(buckets[1].end, 3u);
    EXPECT_TRUE(buckets[1].has_any);
    for (std::size_t i : {2u, 3u}) {
      EXPECT_EQ(buckets[i].begin, 4u);
      EXPECT_EQ(buckets[i].end, 0u);
      EXPECT_FALSE(buckets[i].has_any);
    }
  }
} // namespace

TEST(DispatchMetadata, ResidualSlicesRetainAllCandidateCases) {
  using V    = std::variant<int, std::string, double, bool>;
  auto cases = std::make_tuple(
      is<int>(0) >> 1,
      is<int>(pred([](int value) { return value < 0; })) >> 2,
      is<std::string> >> 3,
      is<int>(pred([](int value) { return value > 0; })) >> 4);
  expect_residual_slices(
      detail::make_typed_variant_case_bucket_table<V,
                                                   decltype(cases),
                                                   4>(
          std::make_index_sequence<4>{}));
}

TEST(DispatchMetadata, DirectTableKeepsFirstDuplicateAlternative) {
  using V     = std::variant<int, std::string, double>;
  using cases = decltype(std::make_tuple(
      is<int> >> 1, alt<0> >> 2, is<std::string> >> 3));
  const std::array<std::uint8_t, 3> expected = {0, 2, 3};
  EXPECT_EQ(
      (detail::make_variant_simple_case_index_table<V,
                                                    cases,
                                                    std::uint8_t>(
          std::make_index_sequence<3>{})),
      expected);
}

TEST(DispatchMetadata, StaticLiteralSummaryCountsKeysAndRange) {
  // Unordered keys, negatives, a duplicate and the default slot.
  using mixed      = decltype(on(val<5> >> 1,
                            val<-3> >> 2,
                            val<2> >> 3,
                            val<-3> >> 4,
                            _ >> 0))::tuple_type;
  using mixed_meta = detail::static_literal_dispatch_metadata<int,
                                                              mixed>;
  static_assert(mixed_meta::literal_case_count == 4);
  static_assert(mixed_meta::has_literal_cases);
  static_assert(mixed_meta::min_value == -3);
  static_assert(mixed_meta::max_value == 5);
  static_assert(mixed_meta::range_size == 9);

  // The ends of an unsigned key.
  using top      = decltype(on(val<std::uint8_t(255)> >> 1,
                          val<std::uint8_t(0)> >> 2,
                          _ >> 0))::tuple_type;
  using top_meta = detail::
      static_literal_dispatch_metadata<std::uint8_t, top>;
  static_assert(top_meta::min_value == 0);
  static_assert(top_meta::max_value == 255);
  static_assert(top_meta::range_size == 256);

  // A bool key, and a runtime literal that is not a static key.
  using flags      = decltype(on(
      val<true> >> 1, val<false> >> 2, _ >> 0))::tuple_type;
  using flags_meta = detail::static_literal_dispatch_metadata<bool,
                                                              flags>;
  static_assert(!flags_meta::min_value && flags_meta::max_value);
  static_assert(flags_meta::range_size == 2);

  using runtime      = decltype(on(
      lit(7) >> 1, val<3> >> 2, _ >> 0))::tuple_type;
  using runtime_meta = detail::
      static_literal_dispatch_metadata<int, runtime>;
  static_assert(runtime_meta::literal_case_count == 1);
  static_assert(runtime_meta::min_value == 3
                && runtime_meta::max_value == 3);

  using none      = decltype(on(lit(7) >> 1, _ >> 0))::tuple_type;
  using none_meta = detail::static_literal_dispatch_metadata<int,
                                                             none>;
  static_assert(none_meta::literal_case_count == 0);
  static_assert(!none_meta::has_literal_cases);
  static_assert(none_meta::range_size == 0);
  SUCCEED();
}

namespace {
  // Calls through a pointer so that the summaries are also computed
  // at run time, not only during constant evaluation.
  template <typename Subject, typename Tuple, std::size_t... I>
  detail::static_literal_summary<
      detail::literal_subject_key_t<Subject>>
  summarize_at_run_time(std::index_sequence<I...> indices) {
    auto *volatile summarize = &detail::summarize_static_literals<
        Subject,
        Tuple,
        I...>;
    return summarize(indices);
  }

  template <typename Subject,
            typename Tuple,
            std::size_t ActiveIndex,
            std::size_t... I>
  detail::variant_case_index_range
  case_range_at_run_time(std::index_sequence<I...> indices) {
    auto *volatile range = &detail::variant_typed_case_range<
        Subject,
        Tuple,
        ActiveIndex,
        I...>;
    return range(indices);
  }
} // namespace

TEST(DispatchMetadata, StaticLiteralSummaryAtRunTime) {
  using mixed        = decltype(on(val<5> >> 1,
                            val<-3> >> 2,
                            lit(9) >> 3,
                            val<2> >> 4,
                            val<-3> >> 5,
                            _ >> 0))::tuple_type;
  const auto summary = summarize_at_run_time<int, mixed>(
      std::make_index_sequence<6>{});
  EXPECT_EQ(summary.count, 4u);
  EXPECT_TRUE(summary.has_any);
  EXPECT_EQ(summary.min, -3);
  EXPECT_EQ(summary.max, 5);

  // Ascending, so each key raises the maximum.
  using rising  = decltype(on(
      val<1> >> 1, val<4> >> 2, _ >> 0))::tuple_type;
  const auto up = summarize_at_run_time<int, rising>(
      std::make_index_sequence<3>{});
  EXPECT_EQ(up.min, 1);
  EXPECT_EQ(up.max, 4);

  using none       = decltype(on(lit(7) >> 1, _ >> 0))::tuple_type;
  const auto empty = summarize_at_run_time<int, none>(
      std::make_index_sequence<2>{});
  EXPECT_EQ(empty.count, 0u);
  EXPECT_FALSE(empty.has_any);
}

TEST(DispatchMetadata, VariantCaseRangeAtRunTime) {
  using V        = std::variant<int, std::string, double, bool>;
  auto all_cases = std::make_tuple(
      is<int>(0) >> 1,
      is<int>(pred([](int value) { return value < 0; })) >> 2,
      is<std::string> >> 3,
      is<int>(pred([](int value) { return value > 0; })) >> 4);
  using cases     = decltype(all_cases);
  const auto ints = case_range_at_run_time<V, cases, 0>(
      std::make_index_sequence<4>{});
  EXPECT_EQ(ints.begin, 0u);
  EXPECT_EQ(ints.end, 4u);
  const auto strings = case_range_at_run_time<V, cases, 1>(
      std::make_index_sequence<4>{});
  EXPECT_EQ(strings.begin, 2u);
  EXPECT_EQ(strings.end, 3u);
  const auto doubles = case_range_at_run_time<V, cases, 2>(
      std::make_index_sequence<4>{});
  EXPECT_EQ(doubles.begin, 4u);
  EXPECT_EQ(doubles.end, 0u);
}
