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
