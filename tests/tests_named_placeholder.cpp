// Tests for PTN_BIND named placeholder macro.
//
// Verifies that named placeholders declared via PTN_BIND produce the
// same types and runtime results as the positional placeholders (_0,
// arg<N>) they replace. All tests exercise the full match pipeline
// (match | on($[guard] >> handler)) to ensure end-to-end
// correctness.

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

  // Declare named placeholders for each struct.
  // These are namespace-scoped inline constexpr arg_t<N> objects.
  PTN_BIND(Point, x, y);
  PTN_BIND(Packet, type, len);
  PTN_BIND(Triple, a, b, c);

} // namespace

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

TEST(NamedPlaceholder, SingleValueGuard) {
  // Same as: $[_0 > 5]
  // Now:    $[x > 5]  (where x maps to arg<0>)
  int  val    = 10;
  auto result = ptn::match(val)
                | PTN_ON(
                    ptn::$[x > 5] >> [](int v) { return v * 2; },
                    ptn::_ >> 0);
  EXPECT_EQ(result, 20);
}

TEST(NamedPlaceholder, SingleValueGuardFails) {
  int  val    = 3;
  auto result = ptn::match(val)
                | PTN_ON(
                    ptn::$[x > 5] >> [](int v) { return v * 2; },
                    ptn::_ >> 0);
  EXPECT_EQ(result, 0);
}

TEST(NamedPlaceholder, StructuralGuardTwoMembers) {
  // Same as:
  //   $(has<&Point::x, &Point::y>)[_0*_0 + arg<1>*arg<1> == 25]
  // Now:
  //   $(has<&Point::x, &Point::y>)[x*x + y*y == 25]
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
// Equivalence: named and positional placeholders produce identical
// results for the same expression.
// =========================================================================

TEST(NamedPlaceholder, NamedEquivalentToPositional) {
  using ptn::_0;
  using ptn::arg;

  Point p{3, 4};

  auto named = ptn::match(p)
               | PTN_ON(
                   ptn::$(ptn::has<&Point::x,
                                   &Point::y>)[x * x + y * y == 25]
                       >> 1,
                   ptn::_ >> 0);

  auto positional = ptn::match(p)
                    | PTN_ON(
                        ptn::$(ptn::has<&Point::x, &Point::y>)
                                [_0 * _0 + arg<1> * arg<1> == 25]
                            >> 1,
                        ptn::_ >> 0);

  EXPECT_EQ(named, positional);
  EXPECT_EQ(named, 1);
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
