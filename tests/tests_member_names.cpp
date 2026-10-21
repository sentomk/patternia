#include <gtest/gtest.h>

#include <ptn/patternia.hpp>

#include <tuple>
#include <type_traits>

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
