// Tests for PTN_BIND named placeholder macro.
//
// Verifies that PTN_BIND declares readable names for bound tuple
// positions. All tests exercise the full match pipeline
// (match | on($[guard] >> handler)) end to end.

#include <gtest/gtest.h>

#include "ptn/patternia.hpp"

// Test fixture structs.
namespace {

  struct Point {
    int x;
    int y;
  };

  struct Packet {
    int type;
    int len;
  };

  struct Triple {
    int a;
    int b;
    int c;
  };

  struct Single {
    int value;
  };

  struct Quad {
    int a;
    int b;
    int c;
    int d;
  };

  struct Penta {
    int a;
    int b;
    int c;
    int d;
    int e;
  };

  // Declare named placeholders for each struct.
  // These are constexpr arg_t<N> objects.
  PTN_BIND(Point, x, y);
  PTN_BIND(Packet, type, len);
  PTN_BIND(Triple, a, b, c);
  PTN_BIND(Single, sv);
  PTN_BIND(Quad, qa, qb, qc, qd);
  PTN_BIND(Penta, pa, pb, pc, pd, pe);

} // namespace

TEST(NamedPlaceholder, SupportsBlockScopeDeclarations) {
  struct Pair {
    int left;
    int right;
  };

  PTN_BIND(Pair, left, right);

  Pair pair{2, 5};
  auto result = ptn::match(pair)
                | ptn::on(
                    ptn::$(ptn::has<&Pair::left,
                                    &Pair::right>)[left < right]
                        >> 1,
                    ptn::_ >> 0);

  EXPECT_EQ(result, 1);
}

// =========================================================================
// Type correctness: PTN_BIND names must be arg_t<N> of the right
// index.
// =========================================================================

TEST(NamedPlaceholder, SingleArgTypeMatchesArg0) {
  static_assert(std::is_same_v<std::decay_t<decltype(x)>,
                               ptn::pat::mod::arg_t<0>>);
}

TEST(NamedPlaceholder, TwoArgTypesMatchPositions) {
  static_assert(std::is_same_v<std::decay_t<decltype(x)>,
                               ptn::pat::mod::arg_t<0>>);
  static_assert(std::is_same_v<std::decay_t<decltype(y)>,
                               ptn::pat::mod::arg_t<1>>);
}

TEST(NamedPlaceholder, ThreeArgTypesMatchPositions) {
  static_assert(std::is_same_v<std::decay_t<decltype(a)>,
                               ptn::pat::mod::arg_t<0>>);
  static_assert(std::is_same_v<std::decay_t<decltype(b)>,
                               ptn::pat::mod::arg_t<1>>);
  static_assert(std::is_same_v<std::decay_t<decltype(c)>,
                               ptn::pat::mod::arg_t<2>>);
}

// =========================================================================
// Runtime correctness: named placeholders in guard expressions.
// =========================================================================

TEST(NamedPlaceholder, SingleValueUsesWildcardPlaceholder) {
  int  val    = 10;
  auto result = ptn::match(val)
                | PTN_ON(
                    ptn::$[ptn::_ > 5] >>
                        [](int v) { return v * 2; },
                    ptn::_ >> 0);
  EXPECT_EQ(result, 20);
}

TEST(NamedPlaceholder, SingleValueWildcardGuardFails) {
  int  val    = 3;
  auto result = ptn::match(val)
                | PTN_ON(
                    ptn::$[ptn::_ > 5] >>
                        [](int v) { return v * 2; },
                    ptn::_ >> 0);
  EXPECT_EQ(result, 0);
}

TEST(NamedPlaceholder, StructuralGuardTwoMembers) {
  Point p{3, 4};
  auto  result = ptn::match(p)
                | PTN_ON(
                    ptn::$(ptn::has<&Point::x,
                                    &Point::y>)[x * x + y * y == 25]
                        >> 1,
                    ptn::_ >> 0);
  EXPECT_EQ(result, 1);
}

TEST(NamedPlaceholder, StructuralGuardFails) {
  Point p{1, 1};
  auto  result = ptn::match(p)
                | PTN_ON(
                    ptn::$(ptn::has<&Point::x,
                                    &Point::y>)[x * x + y * y == 25]
                        >> 1,
                    ptn::_ >> 0);
  EXPECT_EQ(result, 0);
}

TEST(NamedPlaceholder, StructuralGuardLessThan) {
  // x < y
  Point p{3, 7};
  auto  result = ptn::match(p)
                | PTN_ON(
                    ptn::$(ptn::has<&Point::x, &Point::y>)[x < y]
                        >> 1,
                    ptn::_ >> 0);
  EXPECT_EQ(result, 1);
}

TEST(NamedPlaceholder, StructuralGuardEquality) {
  // type == 0x01
  Packet pkt{0x01, 42};
  auto   result = ptn::match(pkt)
                | PTN_ON(
                    ptn::$(ptn::has<&Packet::type>)[type == 1] >> 1,
                    ptn::_ >> 0);
  EXPECT_EQ(result, 1);
}

TEST(NamedPlaceholder, StructuralGuardCompound) {
  // type == 0x01 && len > 0
  Packet pkt{0x01, 42};
  auto   result = ptn::match(pkt)
                | PTN_ON(
                    ptn::$(
                        ptn::has<&Packet::type,
                                 &Packet::len>)[type == 1 && len > 0]
                        >> 1,
                    ptn::_ >> 0);
  EXPECT_EQ(result, 1);
}

TEST(NamedPlaceholder, ThreeMemberGuard) {
  // a + b == c
  Triple t{2, 3, 5};
  auto   result = ptn::match(t)
                | PTN_ON(ptn::$(ptn::has<&Triple::a,
                                         &Triple::b,
                                         &Triple::c>)[a + b == c]
                             >> 1,
                         ptn::_ >> 0);
  EXPECT_EQ(result, 1);
}

TEST(NamedPlaceholder, ThreeMemberGuardFails) {
  Triple t{2, 3, 6};
  auto   result = ptn::match(t)
                | PTN_ON(ptn::$(ptn::has<&Triple::a,
                                         &Triple::b,
                                         &Triple::c>)[a + b == c]
                             >> 1,
                         ptn::_ >> 0);
  EXPECT_EQ(result, 0);
}

// =========================================================================
// Arithmetic expressions in guards (not just comparisons).
// =========================================================================

TEST(NamedPlaceholder, ArithmeticThenCompare) {
  // x + y > 5
  Point p{3, 4};
  auto  result = ptn::match(p)
                | PTN_ON(
                    ptn::$(ptn::has<&Point::x, &Point::y>)[x + y > 5]
                        >> 1,
                    ptn::_ >> 0);
  EXPECT_EQ(result, 1);
}

TEST(NamedPlaceholder, MixedArithmeticAndComparison) {
  // x*x + y*y < 50 && x < y
  Point p{3, 4};
  auto  result = ptn::match(p)
                | PTN_ON(ptn::$(ptn::has<&Point::x, &Point::y>)
                                 [x * x + y * y < 50 && x < y]
                             >> 1,
                         ptn::_ >> 0);
  EXPECT_EQ(result, 1);
}

// =========================================================================
// PTN_BIND arity coverage: 1, 4, and 5 member structs.
// =========================================================================

TEST(NamedPlaceholder, OneMemberTypeCorrect) {
  static_assert(std::is_same_v<std::decay_t<decltype(sv)>,
                               ptn::pat::mod::arg_t<0>>);
}

TEST(NamedPlaceholder, OneMemberGuard) {
  Single s{42};
  auto   result = ptn::match(s)
                | PTN_ON(
                    ptn::$(ptn::has<&Single::value>)[sv > 5] >> 1,
                    ptn::_ >> 0);
  EXPECT_EQ(result, 1);
}

TEST(NamedPlaceholder, OneMemberGuardFails) {
  Single s{3};
  auto   result = ptn::match(s)
                | PTN_ON(
                    ptn::$(ptn::has<&Single::value>)[sv > 5] >> 1,
                    ptn::_ >> 0);
  EXPECT_EQ(result, 0);
}

TEST(NamedPlaceholder, FourMemberTypeCorrect) {
  static_assert(std::is_same_v<std::decay_t<decltype(qa)>,
                               ptn::pat::mod::arg_t<0>>);
  static_assert(std::is_same_v<std::decay_t<decltype(qd)>,
                               ptn::pat::mod::arg_t<3>>);
}

TEST(NamedPlaceholder, FourMemberGuard) {
  // a + b + c == d
  Quad q{1, 2, 3, 6};
  auto result = ptn::match(q)
                | PTN_ON(
                    ptn::$(ptn::has<&Quad::a,
                                    &Quad::b,
                                    &Quad::c,
                                    &Quad::d>)[qa + qb + qc == qd]
                        >> 1,
                    ptn::_ >> 0);
  EXPECT_EQ(result, 1);
}

TEST(NamedPlaceholder, FourMemberGuardFails) {
  Quad q{1, 2, 3, 7};
  auto result = ptn::match(q)
                | PTN_ON(
                    ptn::$(ptn::has<&Quad::a,
                                    &Quad::b,
                                    &Quad::c,
                                    &Quad::d>)[qa + qb + qc == qd]
                        >> 1,
                    ptn::_ >> 0);
  EXPECT_EQ(result, 0);
}

TEST(NamedPlaceholder, FiveMemberTypeCorrect) {
  static_assert(std::is_same_v<std::decay_t<decltype(pa)>,
                               ptn::pat::mod::arg_t<0>>);
  static_assert(std::is_same_v<std::decay_t<decltype(pe)>,
                               ptn::pat::mod::arg_t<4>>);
}

TEST(NamedPlaceholder, FiveMemberGuard) {
  // a * b - c == d + e
  Penta p{6, 2, 5, 4, 3};
  auto  result = ptn::match(p)
                | PTN_ON(
                    ptn::$(
                        ptn::has<&Penta::a,
                                 &Penta::b,
                                 &Penta::c,
                                 &Penta::d,
                                 &Penta::e>)[pa * pb - pc == pd + pe]
                        >> 1,
                    ptn::_ >> 0);
  EXPECT_EQ(result, 1);
}

TEST(NamedPlaceholder, FiveMemberGuardFails) {
  Penta p{6, 2, 5, 4, 4};
  auto  result = ptn::match(p)
                | PTN_ON(
                    ptn::$(
                        ptn::has<&Penta::a,
                                 &Penta::b,
                                 &Penta::c,
                                 &Penta::d,
                                 &Penta::e>)[pa * pb - pc == pd + pe]
                        >> 1,
                    ptn::_ >> 0);
  EXPECT_EQ(result, 0);
}

// =========================================================================
// PTN_BIND arity coverage: 3 and 4 parameter variants.
// =========================================================================

TEST(NamedPlaceholder, BindThreeArgGuard) {
  Triple t{2, 3, 5};
  auto   result = ptn::match(t)
                | PTN_ON(ptn::$(ptn::has<&Triple::a,
                                         &Triple::b,
                                         &Triple::c>)[a + b == c]
                             >> 1,
                         ptn::_ >> 0);
  EXPECT_EQ(result, 1);
}

TEST(NamedPlaceholder, BindThreeArgGuardFails) {
  Triple t{2, 3, 6};
  auto   result = ptn::match(t)
                | PTN_ON(ptn::$(ptn::has<&Triple::a,
                                         &Triple::b,
                                         &Triple::c>)[a + b == c]
                             >> 1,
                         ptn::_ >> 0);
  EXPECT_EQ(result, 0);
}

TEST(NamedPlaceholder, BindFourArgGuard) {
  Quad q{1, 2, 3, 6};
  auto result = ptn::match(q)
                | PTN_ON(
                    ptn::$(ptn::has<&Quad::a,
                                    &Quad::b,
                                    &Quad::c,
                                    &Quad::d>)[qa + qb + qc == qd]
                        >> 1,
                    ptn::_ >> 0);
  EXPECT_EQ(result, 1);
}

TEST(NamedPlaceholder, BindFourArgGuardFails) {
  Quad q{1, 2, 3, 7};
  auto result = ptn::match(q)
                | PTN_ON(
                    ptn::$(ptn::has<&Quad::a,
                                    &Quad::b,
                                    &Quad::c,
                                    &Quad::d>)[qa + qb + qc == qd]
                        >> 1,
                    ptn::_ >> 0);
  EXPECT_EQ(result, 0);
}
