#include <gtest/gtest.h>

#include <ptn/patternia.hpp>

#include <string>
#include <variant>

using namespace ptn;

TEST(PipelineMatch, LitMatchesFirstCase) {
  int x      = 1;
  int result = match(x) | on(lit(1) >> 42, __ >> 0);

  EXPECT_EQ(result, 42);
}

TEST(PipelineMatch, LitFallsToWildcard) {
  int x      = 99;
  int result = match(x) | on(lit(1) >> 42, lit(2) >> 84, __ >> -1);

  EXPECT_EQ(result, -1);
}

TEST(PipelineMatch, LitMultipleCasesSelectsCorrect) {
  int x      = 3;
  int result = match(x) | on(lit(1) >> 10, lit(2) >> 20, lit(3) >> 30, __ >> 0);

  EXPECT_EQ(result, 30);
}

TEST(PipelineMatch, CompileTimeLitMatchesValue) {
  int x      = 2;
  int result = match(x) | on(val<1> >> 10, val<2> >> 20, __ >> 0);

  EXPECT_EQ(result, 20);
}

TEST(PipelineMatch, VoidHandlerSideEffect) {
  int x   = 1;
  int hit = 0;

  match(x) | on(lit(1) >> [&] { hit = 1; }, __ >> [&] { hit = -1; });

  EXPECT_EQ(hit, 1);
}

TEST(PipelineMatch, LambdaHandlerReturnsValue) {
  int x      = 5;
  int result = match(x) | on(lit(5) >> [] { return 500; }, __ >> 0);

  EXPECT_EQ(result, 500);
}

TEST(PipelineMatch, VariantTypeIsDispatch) {
  using V = std::variant<int, std::string>;
  V v     = std::string("hello");

  int result = match(v) | on(is<int>() >> 1, is<std::string>() >> 2, __ >> 0);

  EXPECT_EQ(result, 2);
}

TEST(PipelineMatch, VariantTypeAsBindsValue) {
  using V = std::variant<int, std::string>;
  V v     = 42;

  int result = match(v) | on($(is<int>()) >> [](int i) { return i * 2; }, __ >> 0);

  EXPECT_EQ(result, 84);
}

TEST(PipelineMatch, GuardedBindMatchesWhenTrue) {
  int x      = 10;
  int result =
      match(x) | on($[PTN_LET(value, value > 5)] >> [](int v) { return v; },
                    __ >> 0);

  EXPECT_EQ(result, 10);
}

TEST(PipelineMatch, GuardedBindFallsWhenFalse) {
  int x      = 3;
  int result =
      match(x) | on($[PTN_LET(value, value > 5)] >> [](int v) { return v; },
                    __ >> -1);

  EXPECT_EQ(result, -1);
}

TEST(PipelineMatch, FirstMatchWins) {
  int x      = 1;
  int result = match(x) | on(lit(1) >> 100, lit(1) >> 200, __ >> 0);

  EXPECT_EQ(result, 100);
}

TEST(PipelineMatch, FullyQualifiedWorks) {
  int x      = 1;
  int result = ptn::match(x) | ptn::on(ptn::lit(1) >> 42, ptn::__ >> 0);

  EXPECT_EQ(result, 42);
}

TEST(PipelineMatch, UnderscoreWildcardMatchesAnything) {
  int x      = 999;
  int result = match(x) | on(lit(1) >> 10, _ >> -1);

  EXPECT_EQ(result, -1);
}

TEST(PipelineMatch, UnderscoreWildcardAsOnlyCase) {
  int x      = 42;
  int result = match(x) | on(_ >> 0);

  EXPECT_EQ(result, 0);
}

TEST(PipelineMatch, UnderscoreWildcardWithVoidHandler) {
  int x   = 1;
  int hit = 0;

  match(x) | on(_ >> [&] { hit = 1; });

  EXPECT_EQ(hit, 1);
}

TEST(PipelineMatch, UnderscoreAndDoubleUnderscoreAreEquivalent) {
  int x  = 5;
  int r1 = match(x) | on(lit(5) >> 50, _ >> 0);
  int r2 = match(x) | on(lit(5) >> 50, __ >> 0);

  EXPECT_EQ(r1, r2);
}

TEST(PipelineMatch, DollarBindCapturesSubject) {
  int x      = 42;
  int result = match(x) | on($ >> [](int v) { return v * 2; }, _ >> 0);

  EXPECT_EQ(result, 84);
}

TEST(PipelineMatch, DollarBindWithGuard) {
  int x      = 10;
  int result = match(x) | on($[_0 > 5] >> [](int v) { return v; }, _ >> -1);

  EXPECT_EQ(result, 10);
}

TEST(PipelineMatch, DollarBindGuardRejects) {
  int x      = 3;
  int result = match(x) | on($[_0 > 5] >> [](int v) { return v; }, _ >> -1);

  EXPECT_EQ(result, -1);
}

TEST(PipelineMatch, DollarBindSupportsNamedGuard) {
  int x  = 7;
  int r1 = match(x) | on($[_0 > 0] >> [](int v) { return v; }, _ >> 0);
  int r2 =
      match(x) | on($[PTN_LET(value, value > 0)] >> [](int v) { return v; },
                    _ >> 0);

  EXPECT_EQ(r1, r2);
}

TEST(PipelineMatch, IsVariableTemplateMatchesType) {
  using V = std::variant<int, std::string>;
  V v     = 42;

  int result = match(v) | on(is<int> >> 1, is<std::string> >> 2, _ >> 0);

  EXPECT_EQ(result, 1);
}

TEST(PipelineMatch, AsVariableTemplateBindsValue) {
  using V = std::variant<int, std::string>;
  V v     = 42;

  int result = match(v) | on($(is<int>) >> [](int i) { return i * 2; }, _ >> 0);

  EXPECT_EQ(result, 84);
}

TEST(PipelineMatch, IsWithSubPattern) {
  using V = std::variant<int, std::string>;
  V v     = std::string("hello");

  auto result =
      match(v) | on($(is<std::string>()) >> [](const std::string &s) {
                    return s;
                  },
                    _ >> std::string("other"));

  EXPECT_EQ(result, "hello");
}

TEST(PipelineMatch, AsWithGuard) {
  using V = std::variant<int, std::string>;
  V v     = 42;

  int result = match(v)
               | on($(is<int>())[_0 > 100] >> [](int i) { return i; },
                    $(is<int>()) >> [](int i) { return -i; },
                    _ >> 0);

  EXPECT_EQ(result, -42);
}

TEST(PipelineMatch, AltVariableTemplateMatchesByIndex) {
  using V = std::variant<int, std::string, double>;
  V v     = 3.14;

  int result = match(v) | on(alt<0> >> 1, alt<1> >> 2, alt<2> >> 3, _ >> 0);

  EXPECT_EQ(result, 3);
}

TEST(PipelineMatch, ImplicitLitIntWithLambdaHandler) {
  int x      = 2;
  int result = match(x) | on(1 >> [] { return 10; }, 2 >> [] { return 20; }, _ >> 0);

  EXPECT_EQ(result, 20);
}

TEST(PipelineMatch, ImplicitLitFallsToWildcard) {
  int x      = 99;
  int result = match(x) | on(1 >> [] { return 10; }, 2 >> [] { return 20; }, _ >> -1);

  EXPECT_EQ(result, -1);
}

TEST(PipelineMatch, ImplicitLitEquivalentToExplicit) {
  int x  = 3;
  int r1 = match(x) | on(3 >> [] { return 30; }, _ >> 0);
  int r2 = match(x) | on(lit(3) >> [] { return 30; }, _ >> 0);

  EXPECT_EQ(r1, r2);
}

TEST(PipelineMatch, DollarHasBindsStructuralMembers) {
  struct Point {
    int x;
    int y;
  };
  Point p{3, 4};

  int result = match(p)
               | on($(has<&Point::x, &Point::y>) >> [](int x, int y) {
                    return x + y;
                  },
                    _ >> 0);

  EXPECT_EQ(result, 7);
}

TEST(PipelineMatch, DollarHasWithGuard) {
  struct Point {
    int x;
    int y;
  };
  Point p{3, 4};

  int result = match(p)
               | on($(has<&Point::x, &Point::y>)[arg<0> + arg<1> > 10]
                         >> [](int x, int y) { return x + y; },
                    $(has<&Point::x, &Point::y>) >> [](int x, int y) {
                      return x * y;
                    },
                    _ >> 0);

  EXPECT_EQ(result, 12);
}

TEST(PipelineMatch, DollarHasMatchesStructuralBinding) {
  struct Point {
    int x;
    int y;
  };
  Point p{5, 6};

  int r1 = match(p)
           | on($(has<&Point::x, &Point::y>) >> [](int x, int y) {
                return x + y;
              },
                _ >> 0);

  int r2 = match(p)
           | on($(has<&Point::x, &Point::y>) >> [](int x, int y) {
                return x + y;
              },
                _ >> 0);

  EXPECT_EQ(r1, r2);
}

TEST(PipelineMatch, DollarIsBindsVariantAlternative) {
  using V = std::variant<int, std::string>;
  V v     = 42;

  int result = match(v) | on($(is<int>()) >> [](int i) { return i * 2; }, _ >> 0);

  EXPECT_EQ(result, 84);
}

TEST(PipelineMatch, DollarIsWithGuard) {
  using V = std::variant<int, std::string>;
  V v     = 42;

  int result = match(v)
               | on($(is<int>())[_0 > 100] >> [](int i) { return i; },
                    $(is<int>()) >> [](int i) { return -i; },
                    _ >> 0);

  EXPECT_EQ(result, -42);
}

TEST(PipelineMatch, DollarIsMatchesTypedBinding) {
  using V = std::variant<int, std::string>;
  V v     = 42;

  int r1 = match(v) | on($(is<int>()) >> [](int i) { return i * 2; }, _ >> 0);
  int r2 = match(v) | on($(is<int>()) >> [](int i) { return i * 2; }, _ >> 0);

  EXPECT_EQ(r1, r2);
}

TEST(PipelineMatch, DollarIsWithStringVariant) {
  using V = std::variant<int, std::string>;
  V v     = std::string("hello");

  auto result = match(v)
                | on($(is<std::string>()) >> [](const std::string &s) {
                     return s + " world";
                   },
                     _ >> std::string("other"));

  EXPECT_EQ(result, "hello world");
}
