// `bnd`, the standard spelling of `$`.
//
// This file is also built as `ptn_tests_no_dollar` with
// PTN_NO_DOLLAR defined and, on GCC and Clang,
// -fno-dollars-in-identifiers, which rejects any `$` identifier in
// the headers. Uses of `$` are guarded accordingly.

#include <gtest/gtest.h>

#include <ptn/patternia.hpp>

#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

namespace {

  struct Point {
    int x;
    int y;
  };

} // namespace

TEST(Bnd, BindsTheWholeSubject) {
  using namespace ptn;
  int value = 7;

  int result = match(value) | on(bnd >> [](int v) { return v * 2; });

  EXPECT_EQ(result, 14);
}

TEST(Bnd, GuardsAndFallsThrough) {
  using namespace ptn;
  auto classify = [](int v) {
    return match(v)
           | on(
               bnd[_ > 0] >> [](int x) { return x; },
               lit(0) >> 0,
               _ >> -1);
  };

  EXPECT_EQ(classify(5), 5);
  EXPECT_EQ(classify(0), 0);
  EXPECT_EQ(classify(-3), -1);
}

TEST(Bnd, BindsUnderSubpatterns) {
  using namespace ptn;
  std::variant<int, std::string> v = std::string("abc");

  auto size = match(v)
              | on(
                  bnd(is<std::string>) >>
                      [](const std::string &s) { return s.size(); },
                  _ >> std::size_t{0});
  EXPECT_EQ(size, 3U);

  constexpr auto pt  = has<&Point::x, &Point::y>;
  int            sum = match(Point{3, 4})
            | on(
                bnd(pt) >> [](int x, int y) { return x + y; },
                _ >> 0);
  EXPECT_EQ(sum, 7);
}

TEST(Bnd, EndsTheCaseListLikeDollar) {
  using namespace ptn;
  std::string text = "hi";

  // An unguarded bnd is a complete fallback, so no `_` is needed.
  auto length = match(text)
                | on(
                    lit("") >> std::size_t{0},
                    bnd >> [](const std::string &s) {
                      return s.size();
                    });

  EXPECT_EQ(length, 2U);
}

TEST(Bnd, MovesAnOwnedSubject) {
  using namespace ptn;
  auto owned = std::make_unique<int>(9);

  int result = match(std::move(owned))
               | on(bnd >>
                    [](std::unique_ptr<int> p) { return *p; });

  EXPECT_EQ(result, 9);
}

TEST(Bnd, WorksInStaticCaseLists) {
  int value = 4;

  int result = ptn::match(value)
               | PTN_ON(
                   ptn::bnd[ptn::_ > 3] >> [](int v) { return v; },
                   ptn::_ >> 0);

  EXPECT_EQ(result, 4);
}

#ifndef PTN_NO_DOLLAR
TEST(Bnd, IsTheSameObjectAsDollar) {
  static_assert(std::is_same_v<std::decay_t<decltype(ptn::$)>,
                               std::decay_t<decltype(ptn::bnd)>>);
  EXPECT_EQ(&ptn::$, &ptn::bnd);
}
#endif
