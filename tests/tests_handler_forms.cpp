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
