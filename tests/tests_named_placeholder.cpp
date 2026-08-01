// Tests for PTN_BIND named placeholder macro.
//
// Verifies that PTN_BIND declares member-anchored names for use in
// structural guard expressions. All tests exercise the full match
// pipeline (match | on(pattern[guard] >> handler)) end to end.

#include <gtest/gtest.h>

#include "ptn/patternia.hpp"

#include <variant>

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

  struct Deca {
    int a;
    int b;
    int c;
    int d;
    int e;
    int f;
    int g;
    int h;
    int i;
    int j;
  };

  // Member-anchored placeholders declared at namespace scope.
  // Names must be real members of the given struct, and names in
  // one scope must be unique, so structs with overlapping member
  // names (Triple/Quad/Penta/Deca) bind at block scope inside the
  // tests that need them.
  PTN_BIND(Point, x, y);
  PTN_BIND(Packet, type, len);
  PTN_BIND(Triple, a, b, c);
  PTN_BIND(Single, value);

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
// Type correctness: PTN_BIND names must be
// member_t<&Struct::member>.
// =========================================================================

TEST(NamedPlaceholder, SingleArgTypeMatchesMember) {
  static_assert(std::is_same_v<std::decay_t<decltype(x)>,
                               ptn::pat::mod::member_t<&Point::x>>);
}

TEST(NamedPlaceholder, TwoArgTypesMatchMembers) {
  static_assert(std::is_same_v<std::decay_t<decltype(x)>,
                               ptn::pat::mod::member_t<&Point::x>>);
  static_assert(std::is_same_v<std::decay_t<decltype(y)>,
                               ptn::pat::mod::member_t<&Point::y>>);
}

TEST(NamedPlaceholder, ThreeArgTypesMatchMembers) {
  static_assert(std::is_same_v<std::decay_t<decltype(a)>,
                               ptn::pat::mod::member_t<&Triple::a>>);
  static_assert(std::is_same_v<std::decay_t<decltype(b)>,
                               ptn::pat::mod::member_t<&Triple::b>>);
  static_assert(std::is_same_v<std::decay_t<decltype(c)>,
                               ptn::pat::mod::member_t<&Triple::c>>);
}

// =========================================================================
// Runtime correctness: named placeholders in guard expressions.
// =========================================================================

TEST(NamedPlaceholder, SingleValueUsesWildcardPlaceholder) {
  int  val    = 10;
  auto result = ptn::match(val)
                | ptn::on(
                    ptn::$[ptn::_ > 5] >>
                        [](int v) { return v * 2; },
                    ptn::_ >> 0);
  EXPECT_EQ(result, 20);
}

TEST(NamedPlaceholder, SingleValueWildcardGuardFails) {
  int  val    = 3;
  auto result = ptn::match(val)
                | ptn::on(
                    ptn::$[ptn::_ > 5] >>
                        [](int v) { return v * 2; },
                    ptn::_ >> 0);
  EXPECT_EQ(result, 0);
}

TEST(NamedPlaceholder, StructuralGuardTwoMembers) {
  Point p{3, 4};
  auto  result = ptn::match(p)
                | ptn::on(
                    ptn::$(ptn::has<&Point::x,
                                    &Point::y>)[x * x + y * y == 25]
                        >> 1,
                    ptn::_ >> 0);
  EXPECT_EQ(result, 1);
}

TEST(NamedPlaceholder, StructuralGuardFails) {
  Point p{1, 1};
  auto  result = ptn::match(p)
                | ptn::on(
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
                | ptn::on(
                    ptn::$(ptn::has<&Point::x, &Point::y>)[x < y]
                        >> 1,
                    ptn::_ >> 0);
  EXPECT_EQ(result, 1);
}

TEST(NamedPlaceholder, StructuralGuardEquality) {
  // type == 0x01
  Packet pkt{0x01, 42};
  auto   result = ptn::match(pkt)
                | ptn::on(
                    ptn::$(ptn::has<&Packet::type>)[type == 1] >> 1,
                    ptn::_ >> 0);
  EXPECT_EQ(result, 1);
}

TEST(NamedPlaceholder, StructuralGuardCompound) {
  // type == 0x01 && len > 0
  Packet pkt{0x01, 42};
  auto   result = ptn::match(pkt)
                | ptn::on(
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
                | ptn::on(ptn::$(ptn::has<&Triple::a,
                                          &Triple::b,
                                          &Triple::c>)[a + b == c]
                              >> 1,
                          ptn::_ >> 0);
  EXPECT_EQ(result, 1);
}

TEST(NamedPlaceholder, ThreeMemberGuardFails) {
  Triple t{2, 3, 6};
  auto   result = ptn::match(t)
                | ptn::on(ptn::$(ptn::has<&Triple::a,
                                          &Triple::b,
                                          &Triple::c>)[a + b == c]
                              >> 1,
                          ptn::_ >> 0);
  EXPECT_EQ(result, 0);
}

// =========================================================================
// Member anchoring: names follow members, not positions.
// =========================================================================

TEST(NamedPlaceholder, NamesFollowMembersNotPositions) {
  // has<> lists members in reverse order; names still resolve to
  // their own member. Positionally this guard would read
  // (.y == 3 && .x == 4) and fail.
  Point p{3, 4};
  auto  result = ptn::match(p)
                | ptn::on(
                    ptn::$(ptn::has<&Point::y, &Point::x>)[x == 3
                                                           && y == 4]
                        >> 1,
                    ptn::_ >> 0);
  EXPECT_EQ(result, 1);
}

TEST(NamedPlaceholder, PositionalReadingWouldDiffer) {
  // Same reversed pattern with the positionally-read guard: it
  // must NOT match, proving resolution is by member.
  Point p{3, 4};
  auto  result = ptn::match(p)
                | ptn::on(
                    ptn::$(ptn::has<&Point::y, &Point::x>)[x == 4
                                                           && y == 3]
                        >> 1,
                    ptn::_ >> 0);
  EXPECT_EQ(result, 0);
}

TEST(NamedPlaceholder, MemberNamesSkipIgnoredSlots) {
  // _ign occupies a pattern slot but no binding position, so
  // c resolves to position 1 of the extracted tuple.
  Triple t{2, 99, 5};
  auto   result = ptn::match(t)
                | ptn::on(ptn::$(ptn::has<&Triple::a,
                                          ptn::_ign,
                                          &Triple::c>)[a + c == 7]
                              >> 1,
                          ptn::_ >> 0);
  EXPECT_EQ(result, 1);
}

TEST(NamedPlaceholder, MixedWildcardAndMemberNames) {
  // `_` keeps its positional meaning (position 0 == .x) and can be
  // combined with member-anchored names.
  Point p{3, 4};
  auto  result = ptn::match(p)
                | ptn::on(
                    ptn::$(
                        ptn::has<&Point::x, &Point::y>)[ptn::_ == 3
                                                        && y == 4]
                        >> 1,
                    ptn::_ >> 0);
  EXPECT_EQ(result, 1);
}

TEST(NamedPlaceholder, NonBindingGuardWithMemberNames) {
  // has<>[guard] without binding: handler is nullary, names still
  // anchor to members.
  Packet pkt{0x01, 42};
  auto   result = ptn::match(pkt)
                | ptn::on(
                    ptn::has<&Packet::type, &Packet::len>[type == 1
                                                          && len > 0]
                        >> 1,
                    ptn::_ >> 0);
  EXPECT_EQ(result, 1);
}

TEST(NamedPlaceholder, NonBindingGuardRejects) {
  Packet pkt{0x02, 42};
  auto   result = ptn::match(pkt)
                | ptn::on(
                    ptn::has<&Packet::type, &Packet::len>[type == 1
                                                          && len > 0]
                        >> 1,
                    ptn::_ >> 0);
  EXPECT_EQ(result, 0);
}

TEST(NamedPlaceholder, GuardOnTypePatternWithStructuralSub) {
  // Member names resolve through is<T>(...) wrappers: the guard
  // attaches to the type pattern but anchors to the members of
  // the nested has<...>.
  using V     = std::variant<int, Point>;
  V    v      = Point{3, 4};
  auto result = ptn::match(v)
                | ptn::on(
                    ptn::is<Point>(ptn::$(
                        ptn::has<&Point::x, &Point::y>))[x == 3
                                                         && y == 4]
                        >> [](int px, int py) { return px + py; },
                    ptn::_ >> [] { return 0; });
  EXPECT_EQ(result, 7);
}

TEST(NamedPlaceholder, GuardOnTypePatternRejects) {
  using V     = std::variant<int, Point>;
  V    v      = Point{3, -4};
  auto result = ptn::match(v)
                | ptn::on(
                    ptn::is<Point>(ptn::$(
                        ptn::has<&Point::x, &Point::y>))[x > 0
                                                         && y > 0]
                        >> [](int px, int py) { return px + py; },
                    ptn::_ >> [] { return 0; });
  EXPECT_EQ(result, 0);
}

// =========================================================================
// Arithmetic expressions in guards (not just comparisons).
// =========================================================================

TEST(NamedPlaceholder, ArithmeticThenCompare) {
  // x + y > 5
  Point p{3, 4};
  auto  result = ptn::match(p)
                | ptn::on(
                    ptn::$(ptn::has<&Point::x, &Point::y>)[x + y > 5]
                        >> 1,
                    ptn::_ >> 0);
  EXPECT_EQ(result, 1);
}

TEST(NamedPlaceholder, MixedArithmeticAndComparison) {
  // x*x + y*y < 50 && x < y
  Point p{3, 4};
  auto  result = ptn::match(p)
                | ptn::on(ptn::$(ptn::has<&Point::x, &Point::y>)
                                  [x * x + y * y < 50 && x < y]
                              >> 1,
                          ptn::_ >> 0);
  EXPECT_EQ(result, 1);
}

// =========================================================================
// PTN_BIND arity coverage: 1, 4, and 5 member structs.
// =========================================================================

TEST(NamedPlaceholder, OneMemberTypeCorrect) {
  static_assert(
      std::is_same_v<std::decay_t<decltype(value)>,
                     ptn::pat::mod::member_t<&Single::value>>);
}

TEST(NamedPlaceholder, OneMemberGuard) {
  Single s{42};
  auto   result = ptn::match(s)
                | ptn::on(
                    ptn::$(ptn::has<&Single::value>)[value > 5] >> 1,
                    ptn::_ >> 0);
  EXPECT_EQ(result, 1);
}

TEST(NamedPlaceholder, OneMemberGuardFails) {
  Single s{3};
  auto   result = ptn::match(s)
                | ptn::on(
                    ptn::$(ptn::has<&Single::value>)[value > 5] >> 1,
                    ptn::_ >> 0);
  EXPECT_EQ(result, 0);
}

TEST(NamedPlaceholder, FourMemberTypeCorrect) {
  PTN_BIND(Quad, a, b, c, d);
  static_assert(std::is_same_v<std::decay_t<decltype(a)>,
                               ptn::pat::mod::member_t<&Quad::a>>);
  static_assert(std::is_same_v<std::decay_t<decltype(d)>,
                               ptn::pat::mod::member_t<&Quad::d>>);
}

TEST(NamedPlaceholder, FourMemberGuard) {
  // a + b + c == d
  PTN_BIND(Quad, a, b, c, d);
  Quad q{1, 2, 3, 6};
  auto result = ptn::match(q)
                | ptn::on(ptn::$(ptn::has<&Quad::a,
                                          &Quad::b,
                                          &Quad::c,
                                          &Quad::d>)[a + b + c == d]
                              >> 1,
                          ptn::_ >> 0);
  EXPECT_EQ(result, 1);
}

TEST(NamedPlaceholder, FourMemberGuardFails) {
  PTN_BIND(Quad, a, b, c, d);
  Quad q{1, 2, 3, 7};
  auto result = ptn::match(q)
                | ptn::on(ptn::$(ptn::has<&Quad::a,
                                          &Quad::b,
                                          &Quad::c,
                                          &Quad::d>)[a + b + c == d]
                              >> 1,
                          ptn::_ >> 0);
  EXPECT_EQ(result, 0);
}

TEST(NamedPlaceholder, FiveMemberTypeCorrect) {
  PTN_BIND(Penta, a, b, c, d, e);
  static_assert(std::is_same_v<std::decay_t<decltype(a)>,
                               ptn::pat::mod::member_t<&Penta::a>>);
  static_assert(std::is_same_v<std::decay_t<decltype(e)>,
                               ptn::pat::mod::member_t<&Penta::e>>);
}

TEST(NamedPlaceholder, FiveMemberGuard) {
  // a * b - c == d + e
  PTN_BIND(Penta, a, b, c, d, e);
  Penta p{6, 2, 5, 4, 3};
  auto  result = ptn::match(p)
                | ptn::on(
                    ptn::$(ptn::has<&Penta::a,
                                    &Penta::b,
                                    &Penta::c,
                                    &Penta::d,
                                    &Penta::e>)[a * b - c == d + e]
                        >> 1,
                    ptn::_ >> 0);
  EXPECT_EQ(result, 1);
}

TEST(NamedPlaceholder, FiveMemberGuardFails) {
  PTN_BIND(Penta, a, b, c, d, e);
  Penta p{6, 2, 5, 4, 4};
  auto  result = ptn::match(p)
                | ptn::on(
                    ptn::$(ptn::has<&Penta::a,
                                    &Penta::b,
                                    &Penta::c,
                                    &Penta::d,
                                    &Penta::e>)[a * b - c == d + e]
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
                | ptn::on(ptn::$(ptn::has<&Triple::a,
                                          &Triple::b,
                                          &Triple::c>)[a + b == c]
                              >> 1,
                          ptn::_ >> 0);
  EXPECT_EQ(result, 1);
}

TEST(NamedPlaceholder, BindThreeArgGuardFails) {
  Triple t{2, 3, 6};
  auto   result = ptn::match(t)
                | ptn::on(ptn::$(ptn::has<&Triple::a,
                                          &Triple::b,
                                          &Triple::c>)[a + b == c]
                              >> 1,
                          ptn::_ >> 0);
  EXPECT_EQ(result, 0);
}

TEST(NamedPlaceholder, BindFourArgGuard) {
  PTN_BIND(Quad, a, b, c, d);
  Quad q{1, 2, 3, 6};
  auto result = ptn::match(q)
                | ptn::on(ptn::$(ptn::has<&Quad::a,
                                          &Quad::b,
                                          &Quad::c,
                                          &Quad::d>)[a + b + c == d]
                              >> 1,
                          ptn::_ >> 0);
  EXPECT_EQ(result, 1);
}

TEST(NamedPlaceholder, BindFourArgGuardFails) {
  PTN_BIND(Quad, a, b, c, d);
  Quad q{1, 2, 3, 7};
  auto result = ptn::match(q)
                | ptn::on(ptn::$(ptn::has<&Quad::a,
                                          &Quad::b,
                                          &Quad::c,
                                          &Quad::d>)[a + b + c == d]
                              >> 1,
                          ptn::_ >> 0);
  EXPECT_EQ(result, 0);
}

// =========================================================================
// PTN_BIND arity coverage: 10 member struct (chained expansion).
// =========================================================================

TEST(NamedPlaceholder, TenMemberTypeCorrect) {
  PTN_BIND(Deca, a, b, c, d, e, f, g, h, i, j);
  static_assert(std::is_same_v<std::decay_t<decltype(a)>,
                               ptn::pat::mod::member_t<&Deca::a>>);
  static_assert(std::is_same_v<std::decay_t<decltype(j)>,
                               ptn::pat::mod::member_t<&Deca::j>>);
}

TEST(NamedPlaceholder, TenMemberGuard) {
  // a + b + ... + i == j
  PTN_BIND(Deca, a, b, c, d, e, f, g, h, i, j);
  Deca dc{1, 1, 1, 1, 1, 1, 1, 1, 1, 9};
  auto result = ptn::match(dc)
                | ptn::on(
                    ptn::$(ptn::has<&Deca::a,
                                    &Deca::b,
                                    &Deca::c,
                                    &Deca::d,
                                    &Deca::e,
                                    &Deca::f,
                                    &Deca::g,
                                    &Deca::h,
                                    &Deca::i,
                                    &Deca::j>)[a + b + c + d + e + f
                                                   + g + h + i
                                               == j]
                        >> 1,
                    ptn::_ >> 0);
  EXPECT_EQ(result, 1);
}

TEST(NamedPlaceholder, TenMemberGuardFails) {
  PTN_BIND(Deca, a, b, c, d, e, f, g, h, i, j);
  Deca dc{1, 1, 1, 1, 1, 1, 1, 1, 1, 10};
  auto result = ptn::match(dc)
                | ptn::on(
                    ptn::$(ptn::has<&Deca::a,
                                    &Deca::b,
                                    &Deca::c,
                                    &Deca::d,
                                    &Deca::e,
                                    &Deca::f,
                                    &Deca::g,
                                    &Deca::h,
                                    &Deca::i,
                                    &Deca::j>)[a + b + c + d + e + f
                                                   + g + h + i
                                               == j]
                        >> 1,
                    ptn::_ >> 0);
  EXPECT_EQ(result, 0);
}
