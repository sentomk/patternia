#include <gtest/gtest.h>

#include <ptn/patternia.hpp>

#include <cstdint>
#include <string>
#include <type_traits>
#include <variant>

namespace {

  struct Circle {
    double r;

    double area() const {
      return 3.0 * r * r;
    }
  };

  struct Point {
    int x;
    int y;
  };

  using Shape = std::variant<Circle, int>;
  using Text  = std::variant<int, std::string>;

  double circle_area(const Circle &c) {
    return c.area();
  }

  // A final class cannot be inspected by deriving from it.
  struct final_doubler final {
    int operator()(const int &v) const {
      return v * 2;
    }
  };

  // The body only compiles for std::string, so classifying this
  // handler must not instantiate it with unrelated argument types.
  inline constexpr auto string_size = [](auto &s) {
    return s.size();
  };

  namespace traits = ptn::core::traits::detail;

  enum class Color : std::uint8_t {
    red,
    green
  };

  static_assert(traits::is_handler_like_v<decltype(&Circle::area)>);
  static_assert(traits::is_handler_like_v<decltype(&Point::x)>);
  static_assert(traits::is_handler_like_v<decltype(circle_area)>);
  static_assert(traits::is_handler_like_v<decltype(&circle_area)>);
  static_assert(traits::is_handler_like_v<final_doubler>);
  static_assert(traits::is_handler_like_v<decltype(string_size)>);

  static_assert(!traits::is_handler_like_v<int>);
  static_assert(!traits::is_handler_like_v<double>);
  static_assert(!traits::is_handler_like_v<Color>);
  static_assert(!traits::is_handler_like_v<const char *>);
  static_assert(!traits::is_handler_like_v<std::string>);
  static_assert(!traits::is_handler_like_v<Point>);

} // namespace

TEST(HandlerForms, MemberFunctionPointer) {
  Shape s = Circle{2.0};

  double result = ptn::match(s)
                  | ptn::on(
                      ptn::$(ptn::is<Circle>()) >> &Circle::area,
                      ptn::_ >> -1.0);

  EXPECT_DOUBLE_EQ(result, 12.0);
}

TEST(HandlerForms, MemberDataPointer) {
  Point p{3, 4};

  int result = ptn::match(p)
               | ptn::on(ptn::$ >> &Point::y, ptn::_ >> -1);

  EXPECT_EQ(result, 4);
}

TEST(HandlerForms, FreeFunction) {
  Shape s = Circle{1.0};

  double result = ptn::match(s)
                  | ptn::on(ptn::$(ptn::is<Circle>()) >> circle_area,
                            ptn::_ >> -1.0);

  EXPECT_DOUBLE_EQ(result, 3.0);
}

TEST(HandlerForms, HandlerReturningReference) {
  Point p{3, 4};

  int result = ptn::match(p)
               | ptn::on(
                   ptn::$ >> [](const Point &q) -> const int & {
                     return q.x;
                   },
                   ptn::_ >> -1);

  EXPECT_EQ(result, 3);
}

TEST(HandlerForms, FinalFunctionObject) {
  Text v = 21;

  int result = ptn::match(v)
               | ptn::on(ptn::$(ptn::is<int>()) >> final_doubler{},
                         ptn::_ >> -1);

  EXPECT_EQ(result, 42);
}

TEST(HandlerForms, GenericLambdaWithMemberAccess) {
  Text v = std::string("patternia");

  std::size_t result = ptn::match(v)
                       | ptn::on(ptn::$(ptn::is<std::string>())
                                     >> string_size,
                                 ptn::_ >> std::size_t{0});

  EXPECT_EQ(result, 9u);
}

TEST(HandlerForms, GenericLambdaWrittenInline) {
  Text v = std::string("abc");

  std::size_t result = ptn::match(v)
                       | ptn::on(
                           ptn::$(ptn::is<std::string>()) >>
                               [](auto &s) { return s.size() + 1; },
                           ptn::_ >> std::size_t{0});

  EXPECT_EQ(result, 4u);
}

TEST(HandlerForms, ValuesAreStillValues) {
  Text v = 7;

  EXPECT_EQ(
      ptn::match(v) | ptn::on(ptn::is<int>() >> 10, ptn::_ >> 0),
      10);

  EXPECT_EQ(ptn::match(v)
                | ptn::on(ptn::is<int>() >> Color::green,
                          ptn::_ >> Color::red),
            Color::green);

  EXPECT_STREQ(
      ptn::match(v)
          | ptn::on(ptn::is<int>() >> "seven", ptn::_ >> "other"),
      "seven");

  EXPECT_EQ(ptn::match(v)
                | ptn::on(ptn::is<int>() >> std::string("seven"),
                          ptn::_ >> std::string("other")),
            "seven");
}

// A handler that accepts no arguments ignores the pattern's
// bindings, whether the subject is borrowed or owned.
TEST(HandlerForms, NullaryHandlerIgnoresBindings) {
  int value = 5;

  EXPECT_EQ(ptn::match(value)
                | ptn::on(
                    ptn::$ >> [] { return 1; }, ptn::_ >> 0),
            1);
  EXPECT_EQ(ptn::match(5)
                | ptn::on(
                    ptn::$ >> [] { return 1; }, ptn::_ >> 0),
            1);

  const auto cases = ptn::on(
      ptn::$[ptn::_ > 3] >> [] { return 2; }, ptn::_ >> 0);
  int one = 1;
  EXPECT_EQ(ptn::match(value) | cases, 2);
  EXPECT_EQ(ptn::match(one) | cases, 0);
}

TEST(HandlerForms, NullaryHandlerOnDestructuringPattern) {
  Point p{3, 4};

  int calls = 0;
  ptn::match(p)
      | ptn::on(
          ptn::$(ptn::has<&Point::x, &Point::y>) >> [&] { ++calls; },
          ptn::_ >> [] {});
  EXPECT_EQ(calls, 1);
}

TEST(HandlerForms, NullaryHandlerOnVariantAlternative) {
  Text text = std::string("abc");
  Text num  = 9;

  auto run = [](Text &v) {
    return ptn::match(v)
           | ptn::on(
               ptn::$(ptn::is<std::string>()) >> [] { return 1; },
               ptn::$(ptn::is<int>())[ptn::_ > 3] >>
                   [] { return 2; },
               ptn::_ >> 0);
  };
  EXPECT_EQ(run(text), 1);
  EXPECT_EQ(run(num), 2);
}

TEST(HandlerForms, NullaryHandlerBeforePipe) {
  int value = 5;

  long result = ptn::match(value)
                | ptn::on(
                    ptn::$ >> [] { return 3; } | ptn::to<long>,
                    ptn::_ >> 0L);
  EXPECT_EQ(result, 3L);
}

// to<T> is called with no arguments when T cannot be built from the
// bindings but has a default constructor.
TEST(HandlerForms, ToWithoutMatchingConstructor) {
  int value = 5;

  std::string result = ptn::match(value)
                       | ptn::on(ptn::$ >> ptn::to<std::string>,
                                 ptn::_ >> std::string("other"));
  EXPECT_EQ(result, "");
}
