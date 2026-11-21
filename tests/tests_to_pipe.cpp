#include <gtest/gtest.h>

#include <ptn/patternia.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

using namespace ptn;

namespace {
  struct Point {
    int x;
    int y;
  };

  struct Celsius {
    int degrees;
  };

  struct Empty {};

  class Label {
  public:
    explicit Label(std::string text) : text_(std::move(text)) {
    }
    const std::string &text() const {
      return text_;
    }

  private:
    std::string text_;
  };

  struct Job {
    int id;
  };

  int schedule(std::unique_ptr<Job> job) {
    return job->id;
  }

  int add_hundred(int v) {
    return v + 100;
  }

  struct Started {
    int id;
  };
  struct Stopped {};
  using Event = std::variant<Started, Stopped>;

  namespace dispatch = ptn::core::common::detail;
  namespace engine   = ptn::core::engine::detail;

  constexpr auto point_xy = has<&Point::x, &Point::y>;
} // namespace

TEST(ToPipe, ToBuildsAggregateFromBindings) {
  Point source{3, 4};
  Point copy = match(source)
               | on($(point_xy) >> to<Point>, _ >> Point{0, 0});
  EXPECT_EQ(copy.x, 3);
  EXPECT_EQ(copy.y, 4);

  int value = 7;
  EXPECT_EQ((match(value) | on($ >> to<Celsius>)).degrees, 7);
  static_assert(
      std::is_same_v<decltype(match(value) | on(_ >> to<Empty>)),
                     Empty>);
}

// Non-aggregates use parentheses: vector(3) has three elements,
// where vector{3} would have one.
TEST(ToPipe, ToUsesParenthesesForNonAggregates) {
  int  three = 3;
  auto items = match(three) | on($ >> to<std::vector<int>>);
  EXPECT_EQ(items.size(), 3u);

  std::string text  = "hi";
  Label       label = match(text) | on($ >> to<Label>);
  EXPECT_EQ(label.text(), "hi");
}

TEST(ToPipe, PipePassesHandlerResult) {
  int value = 5;
  EXPECT_EQ(match(value) | on($ >> _ | add_hundred), 105);
  EXPECT_EQ(
      (match(value) | on($ >> (_ * 2 + 1) | to<Celsius>)).degrees,
      11);
  EXPECT_EQ(
      match(value)
          | on($ >> _ | add_hundred | [](int v) { return -v; }),
      -105);

  Point p{3, 4};
  EXPECT_EQ(match(p)
                | on(
                    $(point_xy) >> [](int x, int y) { return x * y; }
                        | add_hundred,
                    _ >> 0),
            112);
}

TEST(ToPipe, PipeAfterValueHandler) {
  int         zero = 0;
  std::string text = match(zero)
                     | on(lit(0) >> "zero" | to<std::string>,
                          _ >> "other" | to<std::string>);
  EXPECT_EQ(text, "zero");
}

TEST(ToPipe, PipeToMemberPointer) {
  Point p{3, 4};
  auto  y = match(p) | on($ >> _ | &Point::y);
  static_assert(std::is_same_v<decltype(y), int>);
  EXPECT_EQ(y, 4);
}

TEST(ToPipe, PipeToVoidCallable) {
  int value = 9;
  int seen  = 0;
  match(value) | on($ >> _ | [&](int v) { seen = v; });
  EXPECT_EQ(seen, 9);
}

// Neither the handler nor the callable after | runs unless the case
// is selected.
TEST(ToPipe, OnlySelectedCaseRuns) {
  int  handler_calls = 0;
  int  pipe_calls    = 0;
  auto cases         = on(
      lit(1) >>
              [&] {
                ++handler_calls;
                return 1;
              }
          |
          [&](int v) {
            ++pipe_calls;
            return v;
          },
      _ >> 0);

  int two = 2;
  EXPECT_EQ(match(two) | cases, 0);
  EXPECT_EQ(handler_calls, 0);
  EXPECT_EQ(pipe_calls, 0);

  int one = 1;
  EXPECT_EQ(match(one) | cases, 1);
  EXPECT_EQ(handler_calls, 1);
  EXPECT_EQ(pipe_calls, 1);
}

TEST(ToPipe, OwnedSubjectMovesIntoPipe) {
  int id = match(std::make_unique<Job>(Job{42}))
           | on($[_ == nullptr] >> -1, $ >> _ | schedule);
  EXPECT_EQ(id, 42);
}

TEST(ToPipe, WithExplicitResultType) {
  int   one   = 1;
  int   two   = 2;
  auto  cases = on<Event>(lit(1) >> 9 | to<Started>,
                         _ >> to<Stopped>);
  Event first = match(one) | cases;
  ASSERT_TRUE(std::holds_alternative<Started>(first));
  EXPECT_EQ(std::get<Started>(first).id, 9);
  EXPECT_TRUE(std::holds_alternative<Stopped>(match(two) | cases));
}

TEST(ToPipe, StatefulPipeKeepsState) {
  auto cases = on(_ >> 1 | [total = 0](int v) mutable {
    total += v;
    return total;
  });

  int value = 0;
  EXPECT_EQ(match(value) | cases, 1);
  EXPECT_EQ(match(value) | cases, 2);

  using cases_t = std::decay_t<decltype(cases)>;
  static_assert(!engine::caches_cases_as_const<cases_t>::value);
}

// A pipe of stateless parts keeps the const cache and the dense
// literal plan.
TEST(ToPipe, StatelessPipeKeepsConstCacheAndPlan) {
  using cases_t = decltype(on(
      val<1> >> 1 | to<long>, val<2> >> 2 | to<long>, _ >> 0L));
  static_assert(engine::caches_cases_as_const<cases_t>::value);
  static_assert(
      dispatch::dispatch_plan<int, cases_t::tuple_type>::kind
      == dispatch::dispatch_plan_kind::static_literal_dense);

  auto run = [](int x) {
    return match(x) | static_on([] {
             return on(val<1> >> 1 | to<long>,
                       val<2> >> 2 | to<long>,
                       _ >> 0L);
           });
  };
  EXPECT_EQ(run(2), 2L);
  EXPECT_EQ(run(7), 0L);
}

TEST(ToPipe, ConstantEvaluation) {
  constexpr long result = [] {
    int value = 2;
    return match(value) | on(lit(2) >> 6 | to<long>, _ >> 0L);
  }();
  static_assert(result == 6L);
  EXPECT_EQ(result, 6L);
}
