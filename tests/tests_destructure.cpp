#include <gtest/gtest.h>

#include <ptn/patternia.hpp>

#include <string>
#include <variant>

using namespace ptn;

struct Point {
  int x;
  int y;
};

struct Packet {
  int         type;
  int         length;
  std::string payload;
};

// -- $(has<>()) destructure binding --

TEST(Destructure, BasicMemberBinding) {
  Point p{3, 4};

  int result = match(p)
               | on($(has<&Point::x, &Point::y>()) >>
                        [](int x, int y) { return x + y; },
                    _ >> 0);

  EXPECT_EQ(result, 7);
}

TEST(Destructure, WithGuard) {
  Point p{5, 10};

  int result = match(p)
               | on($(has<&Point::x, &Point::y>())[_0 > 0 && arg<1> > 0]
                        >> [](int x, int y) { return x * y; },
                    _ >> -1);

  EXPECT_EQ(result, 50);
}

TEST(Destructure, GuardRejects) {
  Point p{-1, 10};

  int result = match(p)
               | on($(has<&Point::x, &Point::y>())[_0 > 0 && arg<1> > 0]
                        >> [](int x, int y) { return x * y; },
                    _ >> -1);

  EXPECT_EQ(result, -1);
}

TEST(Destructure, SingleMember) {
  Point p{42, 0};

  int result = match(p) | on($(has<&Point::x>()) >> [](int x) { return x; }, _ >> 0);

  EXPECT_EQ(result, 42);
}

TEST(Destructure, StructuralBindingProducesMemberValues) {
  Point p{3, 4};

  int r1 = match(p)
           | on($(has<&Point::x, &Point::y>()) >>
                    [](int x, int y) { return x + y; },
                _ >> 0);

  int r2 = match(p)
           | on($(has<&Point::x, &Point::y>()) >>
                    [](int x, int y) { return x + y; },
                _ >> 0);

  EXPECT_EQ(r1, r2);
}

TEST(Destructure, ThreeMembers) {
  Packet pkt{1, 0, "hello"};

  auto result = match(pkt)
                | on($(has<&Packet::type, &Packet::length, &Packet::payload>())
                         >>
                         [](int t, int l, const std::string &p) {
                           return t + l + static_cast<int>(p.size());
                         },
                     _ >> 0);

  EXPECT_EQ(result, 6); // 1 + 0 + 5
}

// -- has<>()[guard] structural verification --

TEST(HasGuard, BasicGuardAccepts) {
  Packet pkt{0x01, 0, ""};

  int result = match(pkt)
               | on(has<&Packet::type>()[_0 == 0x01] >> [] { return 1; },
                    _ >> 0);

  EXPECT_EQ(result, 1);
}

TEST(HasGuard, BasicGuardRejects) {
  Packet pkt{0x02, 0, ""};

  int result = match(pkt)
               | on(has<&Packet::type>()[_0 == 0x01] >> [] { return 1; },
                    _ >> 0);

  EXPECT_EQ(result, 0);
}

TEST(HasGuard, MultiMemberGuard) {
  Packet pkt{0x01, 0, ""};

  int result =
      match(pkt)
      | on(has<&Packet::type, &Packet::length>()[_0 == 0x01 && arg<1> == 0]
               >> [] { return 1; },
           _ >> 0);

  EXPECT_EQ(result, 1);
}

TEST(HasGuard, DoesNotBindToHandler) {
  Packet pkt{0x01, 0, ""};

  int result = match(pkt)
               | on(has<&Packet::type>()[_0 == 0x01] >> [] { return 42; },
                    _ >> 0);

  // Handler takes no arguments — has<> does not bind.
  EXPECT_EQ(result, 42);
}

TEST(HasGuard, FallsThroughOnGuardFail) {
  Packet pkt{0x02, 10, "data"};

  int result = match(pkt)
               | on(has<&Packet::type>()[_0 == 0x01] >> [] { return 1; },
                    has<&Packet::type>()[_0 == 0x02] >> [] { return 2; },
                    _ >> 0);

  EXPECT_EQ(result, 2);
}
