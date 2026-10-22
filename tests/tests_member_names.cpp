#include <gtest/gtest.h>

#include <ptn/patternia.hpp>

#include <tuple>
#include <type_traits>
#include <variant>

namespace {

  struct Point {
    int x;
    int y;
  };

  struct Packet {
    int type;
    int len;
    int crc;
  };

  namespace mod = ptn::pat::mod;

  inline constexpr auto point_pt   = ptn::has<&Point::x, &Point::y>;
  inline constexpr auto swapped_pt = ptn::has<&Point::y, &Point::x>;
  inline constexpr auto
      packet_pt = ptn::has<&Packet::type, ptn::_ign, &Packet::len>;

  using point_pattern_t  = std::decay_t<decltype(point_pt)>;
  using packet_pattern_t = std::decay_t<decltype(packet_pt)>;

  // One element per member, in the order they are listed.
  static_assert(std::tuple_size_v<point_pattern_t> == 2);
  static_assert(
      std::is_same_v<std::tuple_element_t<0, point_pattern_t>,
                     mod::member_t<&Point::x>>);
  static_assert(
      std::is_same_v<std::tuple_element_t<1, point_pattern_t>,
                     mod::member_t<&Point::y>>);

  // `_ign` slots do not get a name.
  static_assert(std::tuple_size_v<packet_pattern_t> == 2);
  static_assert(std::tuple_size_v<std::decay_t<decltype(ptn::has<>)>>
                == 0);
  static_assert(
      std::tuple_size_v<
          std::decay_t<decltype(ptn::has<ptn::_ign, ptn::_ign>)>>
      == 0);
  static_assert(
      std::is_same_v<std::tuple_element_t<0, packet_pattern_t>,
                     mod::member_t<&Packet::type>>);
  static_assert(
      std::is_same_v<std::tuple_element_t<1, packet_pattern_t>,
                     mod::member_t<&Packet::len>>);

  // Names at namespace scope can be used inside PTN_ON.
  auto [px, py]         = point_pt;
  auto [sy, sx]         = swapped_pt;
  auto [pkt_type, plen] = packet_pt;

} // namespace

TEST(MemberNames, NamespaceScopeNamesWorkInsidePtnOn) {
  auto classify = [](const Point &p) {
    return ptn::match(p)
           | PTN_ON(ptn::$(point_pt)[px * px + py * py == 25] >> 1,
                    ptn::_ >> 0);
  };

  EXPECT_EQ(classify(Point{3, 4}), 1);
  EXPECT_EQ(classify(Point{1, 1}), 0);
}

TEST(MemberNames, LocalNamesWorkWithOn) {
  // Automatic block-scope bindings cannot be used in PTN_ON's
  // captureless lambda. Use on, or static bindings in C++20.
  constexpr auto pt = ptn::has<&Point::x, &Point::y>;
  auto [a, b]       = pt;

  Point p{3, 4};

  int result = ptn::match(p)
               | ptn::on(ptn::$(pt)[a * a + b * b == 25] >> 1,
                         ptn::_ >> 0);

  EXPECT_EQ(result, 1);
}

#if __cplusplus >= 202002L                                          \
    || (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L)
TEST(MemberNames, LocalStaticNamesWorkInsidePtnOn) {
  static constexpr auto pt = ptn::has<&Point::x, &Point::y>;
  static auto [a, b]       = pt;

  auto classify = [](const Point &p) {
    return ptn::match(p)
           | PTN_ON(ptn::$(pt)[a * a + b * b == 25] >> 1,
                    ptn::_ >> 0);
  };

  EXPECT_EQ(classify(Point{3, 4}), 1);
  EXPECT_EQ(classify(Point{1, 1}), 0);
}
#endif

TEST(MemberNames, NamesFollowMembersNotPositions) {
  Point p{3, 4};

  // swapped_pt lists y first, so `sy` is still .y and `sx` is .x.
  int result = ptn::match(p)
               | ptn::on(ptn::$(swapped_pt)[sy == 4 && sx == 3] >> 1,
                         ptn::_ >> 0);

  EXPECT_EQ(result, 1);
}

TEST(MemberNames, IgnoredMembersAreSkipped) {
  Packet ping{1, 0, 99};
  Packet data{1, 8, 99};

  auto is_ping = [](const Packet &pkt) {
    return ptn::match(pkt)
           | ptn::on(
               ptn::$(packet_pt)[pkt_type == 1 && plen == 0] >> true,
               ptn::_ >> false);
  };

  EXPECT_TRUE(is_ping(ping));
  EXPECT_FALSE(is_ping(data));
}

TEST(MemberNames, GuardedBindingStillReachesTheHandler) {
  Point p{1, 2};

  int result = ptn::match(p)
               | ptn::on(
                   ptn::$(point_pt)[px < py] >>
                       [](int x, int y) { return x * 10 + y; },
                   ptn::_ >> -1);

  EXPECT_EQ(result, 12);
}

TEST(MemberNames, RuntimeThresholdsAreStoredByValue) {
  // Build matchers from runtime configuration. Each must retain
  // its own threshold, even after the configuration changes.
  auto make_matcher = [](const int &minimum) {
    return ptn::on(
        ptn::$(swapped_pt)[px + py >= minimum] >>
            [](int y, int x) { return x * 10 + y; },
        ptn::_ >> -1);
  };

  int  minimum = 7;
  auto lower   = make_matcher(minimum);
  minimum      = 8;
  auto higher  = make_matcher(minimum);
  minimum      = 0;
  auto zero    = make_matcher(minimum);

  Point point{3, 4};
  EXPECT_EQ(ptn::match(point) | lower, 34);
  EXPECT_EQ(ptn::match(point) | higher, -1);
  point.y = 5;
  EXPECT_EQ(ptn::match(point) | higher, 35);
  point = {2, 4};
  EXPECT_EQ(ptn::match(point) | lower, -1);
  EXPECT_EQ(ptn::match(point) | zero, 24);
}

TEST(MemberNames, ComposedRuntimeGuardsPreserveShortCircuiting) {
  constexpr auto pt = ptn::
      has<ptn::_ign, &Point::y, ptn::_ign, &Point::x, ptn::_ign>;
  int  calls          = 0;
  int  fallback_calls = 0;
  auto ordered        = [&calls](int y, int x) {
    ++calls;
    return x < y;
  };
  auto fallback = [&fallback_calls](int, int) {
    ++fallback_calls;
    return true;
  };
  auto make_matcher = [&](int minimum, int accepted_y) {
    return ptn::on(
        ptn::$(pt)[(px >= minimum && ordered)
                   || (py == accepted_y && fallback)]
            >> [](int y, int x) { return x * 10 + y; },
        ptn::_ >> -1);
  };
  auto matcher  = make_matcher(3, 9);
  auto classify = [&](const Point &point) {
    return ptn::match(point) | matcher;
  };

  // A false left operand skips the callable; the right-hand member
  // expression still resolves through the reordered, ignored slots.
  EXPECT_EQ(classify(Point{2, 4}), -1);
  EXPECT_EQ(calls, 0);
  EXPECT_EQ(fallback_calls, 0);
  EXPECT_EQ(classify(Point{2, 9}), 29);
  EXPECT_EQ(calls, 0);
  EXPECT_EQ(fallback_calls, 1);
  // Both expressions accept this point, but OR skips the fallback.
  EXPECT_EQ(classify(Point{3, 9}), 39);
  EXPECT_EQ(calls, 1);
  EXPECT_EQ(fallback_calls, 1);
  EXPECT_EQ(classify(Point{5, 4}), -1);
  EXPECT_EQ(calls, 2);
  EXPECT_EQ(fallback_calls, 1);
}

TEST(MemberNames, NonBindingRuntimeGuardKeepsHandlerNullary) {
  auto make_matcher = [](int expected_type, int maximum_length) {
    return ptn::on(
        packet_pt[pkt_type == expected_type
                  && plen <= maximum_length]
            >> [] { return true; },
        ptn::_ >> false);
  };
  auto matcher  = make_matcher(2, 8);
  auto classify = [&](const Packet &packet) {
    return ptn::match(packet) | matcher;
  };

  EXPECT_TRUE(classify(Packet{2, 8, 99}));
  EXPECT_FALSE(classify(Packet{2, 9, 99}));
  EXPECT_FALSE(classify(Packet{1, 8, 99}));
}

TEST(MemberNames, RuntimeGuardResolvesThroughVariantWrapper) {
  using subject_t   = std::variant<int, Point>;
  int  calls        = 0;
  auto make_matcher = [&](int minimum) {
    return ptn::on(
        ptn::is<Point>(ptn::$(swapped_pt))[px + py >= minimum] >>
            [&calls](int y, int x) {
              ++calls;
              return x * 10 + y;
            },
        ptn::_ >> -1);
  };
  auto matcher  = make_matcher(7);
  auto classify = [&](const subject_t &subject) {
    return ptn::match(subject) | matcher;
  };

  EXPECT_EQ(classify(subject_t{Point{3, 4}}), 34);
  EXPECT_EQ(calls, 1);
  EXPECT_EQ(classify(subject_t{Point{1, 2}}), -1);
  EXPECT_EQ(classify(subject_t{42}), -1);
  EXPECT_EQ(calls, 1);
}
