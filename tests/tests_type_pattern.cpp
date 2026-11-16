#include <gtest/gtest.h>

#include <ptn/patternia.hpp>

#include <stdexcept>
#include <string>
#include <utility>
#include <variant>

using namespace ptn;

TEST(TypePattern, TypeIsAndTypeAs) {
  std::variant<int, std::string> v = std::string("patternia");

  int result = ptn::match(v)
               | ptn::on(
                   ptn::is<int>() >> [] { return -1; },
                   ptn::$(ptn::is<std::string>()) >>
                       [](const std::string &s) {
                         return static_cast<int>(s.size());
                       },
                   ptn::_ >> 0);

  EXPECT_EQ(result, 9);
}

TEST(TypePattern, AltByIndex) {
  std::variant<int, std::string> v = 42;

  const char *result = ptn::match(v)
                       | ptn::on(ptn::alt<0>() >> "int",
                                 ptn::alt<1>() >> "string",
                                 ptn::_ >> "other");

  EXPECT_STREQ(result, "int");
}

TEST(TypePattern, SimpleVariantDispatchPreservesFirstMatchWins) {
  std::variant<int, std::string> v = 42;

  int hit_count = 0;
  int result    = ptn::match(v)
               | ptn::on(
                   ptn::is<int>() >>
                       [&] {
                         ++hit_count;
                         return 1;
                       },
                   ptn::is<int>() >>
                       [&] {
                         ++hit_count;
                         return 2;
                       },
                   ptn::_ >>
                       [&] {
                         ++hit_count;
                         return 3;
                       });

  EXPECT_EQ(result, 1);
  EXPECT_EQ(hit_count, 1);
}

TEST(TypePattern, SimpleVariantDispatchFallsBackToWildcard) {
  std::variant<int, std::string> v = std::string("patternia");

  int result = ptn::match(v)
               | ptn::on(ptn::is<int>() >> 1, ptn::_ >> 99);

  EXPECT_EQ(result, 99);
}

TEST(TypePattern, SimpleVariantDispatchUnlistedAltFallsToWildcard) {
  std::variant<int, std::string, double> v = 3.14;

  int wildcard_hits = 0;
  int int_hits      = 0;
  int str_hits      = 0;

  int result = ptn::match(v)
               | ptn::on(
                   ptn::is<int>() >>
                       [&] {
                         ++int_hits;
                         return 1;
                       },
                   ptn::is<std::string>() >>
                       [&] {
                         ++str_hits;
                         return 2;
                       },
                   ptn::_ >>
                       [&] {
                         ++wildcard_hits;
                         return 7;
                       });

  EXPECT_EQ(result, 7);
  EXPECT_EQ(wildcard_hits, 1);
  EXPECT_EQ(int_hits, 0);
  EXPECT_EQ(str_hits, 0);
}

TEST(TypePattern, MixedVariantGuardedFallsThroughToSimpleCase) {
  std::variant<int, std::string> v = 42;

  int guarded_hits = 0;
  int simple_hits  = 0;

  int result = ptn::match(v)
               | ptn::on(
                   ptn::$(ptn::is<int>())[ptn::_ > 100] >>
                       [&](int) {
                         ++guarded_hits;
                         return 10;
                       },
                   ptn::is<int>() >>
                       [&] {
                         ++simple_hits;
                         return 1;
                       },
                   ptn::_ >> [] { return 0; });

  EXPECT_EQ(result, 1);
  EXPECT_EQ(guarded_hits, 0);
  EXPECT_EQ(simple_hits, 1);
}

TEST(TypePattern, MixedVariantGuardedCaseWinsWhenPredicateTrue) {
  std::variant<int, std::string> v = std::string("patternia");
  auto long_string                 = [](const std::string &s) {
    return s.size() > 4;
  };

  int guarded_hits = 0;
  int simple_hits  = 0;

  int result = ptn::match(v)
               | ptn::on(
                   ptn::$(ptn::is<std::string>())[long_string] >>
                       [&](const std::string &) {
                         ++guarded_hits;
                         return 20;
                       },
                   ptn::is<std::string>() >>
                       [&] {
                         ++simple_hits;
                         return 2;
                       },
                   ptn::_ >> [] { return 0; });

  EXPECT_EQ(result, 20);
  EXPECT_EQ(guarded_hits, 1);
  EXPECT_EQ(simple_hits, 0);
}

// --- alt<I> and is<T>: cover both the no-subpattern fast path and
// the subpattern path for match()/bind() in type.hpp. ---

TEST(TypePattern, AltByIndexNoSubpatternMatchFastPath) {
  std::variant<int, std::string> v = std::string("x");
  // alt<I>() with no subpattern: match() fast-path returns true;
  // handler is nullary (no_subpattern binds nothing).
  auto r = ptn::match(v)
           | ptn::on(ptn::alt<1>() >> [] { return 1; }, ptn::_ >> 0);
  EXPECT_EQ(r, 1);
}

TEST(TypePattern, AltByIndexWithSubpattern) {
  std::variant<int, std::string> v = std::string("hi");
  // alt<I>(sub): the subpattern match()/bind() branches.
  auto r = ptn::match(v)
           | ptn::on(ptn::alt<1>(ptn::pred([](const std::string &s) {
                       return s.size() == 2;
                     })) >> 1,
                     ptn::_ >> 0);
  EXPECT_EQ(r, 1);
}

TEST(TypePattern, AltRejectsWrongIndex) {
  std::variant<int, std::string> v = 7;
  auto r = ptn::match(v) | ptn::on(ptn::alt<1>() >> 1, ptn::_ >> 0);
  EXPECT_EQ(r, 0);
}

TEST(TypePattern, IsWithSubpatternMatchAndBind) {
  struct P {
    int a;
    int b;
  };
  std::variant<int, P> v = P{2, 3};
  // is<T>(sub): exercises the subpattern branch of type_is_pattern.
  auto r = ptn::match(v)
           | ptn::on(
               ptn::is<P>(ptn::$(ptn::has<&P::a, &P::b>)) >>
                   [](int a, int b) { return a * b; },
               ptn::_ >> 0);
  EXPECT_EQ(r, 6);
}

TEST(TypePattern, PlainValueSubpattern) {
  using V = std::variant<int, std::string>;

  auto run = [](const V &v) {
    return match(v) | on(is<int>(0) >> 1, alt<1>("hi") >> 2, _ >> 0);
  };

  EXPECT_EQ(run(V{0}), 1);
  EXPECT_EQ(run(V{std::string("hi")}), 2);
  EXPECT_EQ(run(V{5}), 0);
  EXPECT_EQ(run(V{std::string("bye")}), 0);
}

TEST(TypePattern, BoundPlainValueSubpatternBindsTheAlternative) {
  std::variant<int, std::string> v = 0;

  int r = match(v)
          | on(
              $(is<int>(0)) >> [](int i) { return i + 7; }, _ >> -1);

  EXPECT_EQ(r, 7);
}

// A variant left valueless by an exception has no active
// alternative. A borrowed match must still reach its fallback,
// whichever dispatch shape the planner picks for the case list.
namespace {
  struct ThrowsOnMove {
    ThrowsOnMove() = default;
    // Deliberately throw to create a valueless variant for the test.
    // NOLINTNEXTLINE(bugprone-unsafe-to-allow-exceptions)
    ThrowsOnMove(ThrowsOnMove &&) noexcept(false) {
      throw std::runtime_error("test");
    }
    ThrowsOnMove &operator=(ThrowsOnMove &&) = default;
  };

  template <typename V>
  V make_valueless() {
    V source(std::in_place_type<ThrowsOnMove>);
    V subject;
    EXPECT_THROW(subject = std::move(source), std::runtime_error);
    return subject;
  }

  struct A0 {};
  struct A1 {};
  struct A2 {};
  struct A3 {};
  struct A4 {};
  struct A5 {};
  struct A6 {};
  struct A7 {};
  struct A8 {};
} // namespace

TEST(TypePattern, ValuelessVariantReachesWildcardFallback) {
  using V   = std::variant<int, ThrowsOnMove>;
  V subject = make_valueless<V>();
  ASSERT_TRUE(subject.valueless_by_exception());

  EXPECT_EQ(
      match(subject)
          | on(is<int>() >> 1, is<ThrowsOnMove>() >> 2, _ >> 3),
      3);
}

TEST(TypePattern, ValuelessVariantReachesBindingFallback) {
  using V   = std::variant<int, ThrowsOnMove>;
  V subject = make_valueless<V>();
  ASSERT_TRUE(subject.valueless_by_exception());

  EXPECT_EQ(match(subject)
                | on(
                    $(is<int>()) >> [](int i) { return i; },
                    is<ThrowsOnMove>() >> 2,
                    $ >>
                        [](const V &v) {
                          return v.valueless_by_exception() ? 3 : 4;
                        }),
            3);
}

TEST(TypePattern, ValuelessVariantSkipsGuardedTypeCases) {
  using V   = std::variant<int, ThrowsOnMove>;
  V subject = make_valueless<V>();
  ASSERT_TRUE(subject.valueless_by_exception());

  EXPECT_EQ(match(subject)
                | on(
                    $(is<int>())[_ > 0] >> [](int i) { return i; },
                    $(is<int>()) >> [](int) { return 1; },
                    is<ThrowsOnMove>() >> 2,
                    _ >> 3),
            3);
}

TEST(TypePattern, ValuelessVariantWithManyAlternatives) {
  using V = std::
      variant<A0, A1, A2, A3, A4, A5, A6, A7, A8, ThrowsOnMove>;
  V subject = make_valueless<V>();
  ASSERT_TRUE(subject.valueless_by_exception());

  EXPECT_EQ(match(subject)
                | on(is<A0>() >> 0,
                     is<A1>() >> 1,
                     is<A2>() >> 2,
                     is<A3>() >> 3,
                     is<A4>() >> 4,
                     is<A5>() >> 5,
                     is<A6>() >> 6,
                     is<A7>() >> 7,
                     is<A8>() >> 8,
                     is<ThrowsOnMove>() >> 9,
                     _ >> -1),
            -1);
}

// `$(alt<I>)` binds the selected alternative, like `$(is<T>)`, for
// borrowed and owned subjects alike. It used to bind the whole
// variant when the subject was borrowed.
namespace {
  using AltSubject = std::variant<int, std::string>;
} // namespace

TEST(TypePattern, DollarAltBindsTheAlternative) {
  const AltSubject i(7);
  const AltSubject s(std::string("hey"));

  auto run = [](const AltSubject &v) {
    return match(v)
           | on(
               $(alt<0>) >> [](int x) { return x; },
               $(alt<1>()) >>
                   [](const std::string &t) {
                     return static_cast<int>(t.size()) * 100;
                   },
               _ >> -1);
  };
  EXPECT_EQ(run(i), 7);
  EXPECT_EQ(run(s), 300);
}

TEST(TypePattern, DollarAltBindsAReferenceToTheActiveAlternative) {
  const AltSubject   v(std::string("abc"));
  const std::string *seen = nullptr;
  match(v)
      | on(
          $(alt<1>) >> [&](const std::string &t) { seen = &t; },
          _ >> [] {});
  EXPECT_EQ(seen, std::get_if<1>(&v));
}

TEST(TypePattern, DollarAltGuardSeesTheAlternative) {
  auto run = [](const AltSubject &v) {
    return match(v)
           | on(
               $(alt<0>)[_ > 5] >> [](int x) { return x * 10; },
               $(alt<0>) >> [](int x) { return x; },
               _ >> -1);
  };
  EXPECT_EQ(run(AltSubject(7)), 70);
  EXPECT_EQ(run(AltSubject(3)), 3);
  EXPECT_EQ(run(AltSubject(std::string("x"))), -1);
}

TEST(TypePattern, DollarAltAgreesWithDollarIsAndOwnedSubjects) {
  auto by_index = [](const AltSubject &v) {
    return match(v)
           | on(
               $(alt<0>(0)) >> [](int x) { return x + 1; },
               $(alt<0>) >> [](int x) { return x * 2; },
               _ >> -1);
  };
  auto by_type = [](const AltSubject &v) {
    return match(v)
           | on(
               $(is<int>(0)) >> [](int x) { return x + 1; },
               $(is<int>) >> [](int x) { return x * 2; },
               _ >> -1);
  };
  auto owned = [](AltSubject v) {
    return match(std::move(v))
           | on(
               $(alt<0>(0)) >> [](int x) { return x + 1; },
               $(alt<0>) >> [](int x) { return x * 2; },
               _ >> -1);
  };
  for (const AltSubject &v : {AltSubject(0),
                              AltSubject(4),
                              AltSubject(std::string("s"))}) {
    EXPECT_EQ(by_index(v), by_type(v));
    EXPECT_EQ(by_index(v), owned(v));
  }
}

TEST(TypePattern, DollarAltSelectsByIndexWithRepeatedTypes) {
  using Twice = std::variant<int, int>;
  auto run    = [](const Twice &v) {
    return match(v)
           | on(
               $(alt<1>) >> [](int x) { return 100 + x; },
               $(alt<0>) >> [](int x) { return x; },
               _ >> -1);
  };
  EXPECT_EQ(run(Twice(std::in_place_index<0>, 3)), 3);
  EXPECT_EQ(run(Twice(std::in_place_index<1>, 3)), 103);
}

TEST(TypePattern, DollarAltWithManyAlternatives) {
  using V  = std::variant<A0, A1, A2, A3, A4, A5, A6, A7, A8, int>;
  auto run = [](const V &v) {
    return match(v)
           | on(
               $(alt<9>)[_ > 0] >> [](int x) { return x; },
               $(alt<0>) >> [](const A0 &) { return 0; },
               _ >> -1);
  };
  EXPECT_EQ(run(V(std::in_place_index<9>, 42)), 42);
  EXPECT_EQ(run(V(std::in_place_index<9>, -1)), -1);
  EXPECT_EQ(run(V(std::in_place_index<0>)), 0);
  EXPECT_EQ(run(V(std::in_place_index<3>)), -1);
}
