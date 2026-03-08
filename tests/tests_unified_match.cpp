#include <gtest/gtest.h>

#include <ptn/patternia.hpp>

#include <string>
#include <variant>

using namespace ptn;

// -- Basic literal matching via unified entry --

TEST(UnifiedMatch, LitMatchesFirstCase) {
  int x      = 1;
  int result = match(x, lit(1) >> 42, __ >> 0);

  EXPECT_EQ(result, 42);
}

TEST(UnifiedMatch, LitFallsToWildcard) {
  int x      = 99;
  int result = match(x, lit(1) >> 42, lit(2) >> 84, __ >> -1);

  EXPECT_EQ(result, -1);
}

TEST(UnifiedMatch, LitMultipleCasesSelectsCorrect) {
  int x      = 3;
  int result = match(
      x, lit(1) >> 10, lit(2) >> 20, lit(3) >> 30, __ >> 0);

  EXPECT_EQ(result, 30);
}

// -- Compile-time literal matching --

TEST(UnifiedMatch, CompileTimeLitMatchesValue) {
  int x      = 2;
  int result = match(x, lit<1>() >> 10, lit<2>() >> 20, __ >> 0);

  EXPECT_EQ(result, 20);
}

// -- Void handler (side-effect only) --

TEST(UnifiedMatch, VoidHandlerSideEffect) {
  int x   = 1;
  int hit = 0;

  match(x, lit(1) >> [&] { hit = 1; }, __ >> [&] { hit = -1; });

  EXPECT_EQ(hit, 1);
}

// -- Lambda handler with return --

TEST(UnifiedMatch, LambdaHandlerReturnsValue) {
  int x      = 5;
  int result = match(x, lit(5) >> [] { return 500; }, __ >> 0);

  EXPECT_EQ(result, 500);
}

// -- Variant type dispatch --

TEST(UnifiedMatch, VariantTypeIsDispatch) {
  using V = std::variant<int, std::string>;
  V v     = std::string("hello");

  int result = match(v,
                     type::is<int>() >> 1,
                     type::is<std::string>() >> 2,
                     __ >> 0);

  EXPECT_EQ(result, 2);
}

TEST(UnifiedMatch, VariantTypeAsBindsValue) {
  using V = std::variant<int, std::string>;
  V v     = 42;

  int result = match(
      v, type::as<int>() >> [](int i) { return i * 2; }, __ >> 0);

  EXPECT_EQ(result, 84);
}

// -- Guarded binding --

TEST(UnifiedMatch, GuardedBindMatchesWhenTrue) {
  int x      = 10;
  int result = match(
      x, bind()[_0 > 5] >> [](int v) { return v; }, __ >> 0);

  EXPECT_EQ(result, 10);
}

TEST(UnifiedMatch, GuardedBindFallsWhenFalse) {
  int x      = 3;
  int result = match(
      x, bind()[_0 > 5] >> [](int v) { return v; }, __ >> -1);

  EXPECT_EQ(result, -1);
}

// -- First match wins --

TEST(UnifiedMatch, FirstMatchWins) {
  int x      = 1;
  int result = match(x, lit(1) >> 100, lit(1) >> 200, __ >> 0);

  EXPECT_EQ(result, 100);
}

// -- Fully qualified (no using directive) --

TEST(UnifiedMatch, FullyQualifiedWorks) {
  int x      = 1;
  int result = ptn::match(x, ptn::lit(1) >> 42, ptn::__ >> 0);

  EXPECT_EQ(result, 42);
}

// -- _ wildcard alias --

TEST(UnifiedMatch, UnderscoreWildcardMatchesAnything) {
  int x      = 999;
  int result = match(x, lit(1) >> 10, _ >> -1);

  EXPECT_EQ(result, -1);
}

TEST(UnifiedMatch, UnderscoreWildcardAsOnlyCase) {
  int x      = 42;
  int result = match(x, _ >> 0);

  EXPECT_EQ(result, 0);
}

TEST(UnifiedMatch, UnderscoreWildcardWithVoidHandler) {
  int x   = 1;
  int hit = 0;

  match(x, _ >> [&] { hit = 1; });

  EXPECT_EQ(hit, 1);
}

TEST(UnifiedMatch, UnderscoreAndDoubleUnderscoreAreEquivalent) {
  int x  = 5;
  int r1 = match(x, lit(5) >> 50, _ >> 0);
  int r2 = match(x, lit(5) >> 50, __ >> 0);

  EXPECT_EQ(r1, r2);
}

// -- $ bind shorthand --

TEST(UnifiedMatch, DollarBindCapturesSubject) {
  int x      = 42;
  int result = match(x, $ >> [](int v) { return v * 2; }, _ >> 0);

  EXPECT_EQ(result, 84);
}

TEST(UnifiedMatch, DollarBindWithGuard) {
  int x      = 10;
  int result = match(
      x, $[_0 > 5] >> [](int v) { return v; }, _ >> -1);

  EXPECT_EQ(result, 10);
}

TEST(UnifiedMatch, DollarBindGuardRejects) {
  int x      = 3;
  int result = match(
      x, $[_0 > 5] >> [](int v) { return v; }, _ >> -1);

  EXPECT_EQ(result, -1);
}

TEST(UnifiedMatch, DollarBindEquivalentToBind) {
  int x  = 7;
  int r1 = match(x, $[_0 > 0] >> [](int v) { return v; }, _ >> 0);
  int r2 = match(
      x, bind()[_0 > 0] >> [](int v) { return v; }, _ >> 0);

  EXPECT_EQ(r1, r2);
}
