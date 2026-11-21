#include <gtest/gtest.h>

#include <ptn/patternia.hpp>

#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <variant>

using namespace ptn;

namespace {
  struct Started {
    int id;
  };
  struct Stopped {};
  using Transition = std::variant<Started, Stopped>;

  enum class Action : std::uint8_t {
    start,
    stop,
    pause
  };

  struct Base {
    Base()                        = default;
    Base(const Base &)            = delete;
    Base &operator=(const Base &) = delete;
    virtual ~Base()               = default;
    virtual int id() const {
      return 1;
    }
  };

  struct Derived : Base {
    int id() const override {
      return 2;
    }
  };

  namespace dispatch = ptn::core::common::detail;

  template <typename Subject, typename Cases>
  constexpr auto plan_kind_v = dispatch::
      dispatch_plan<Subject, typename Cases::tuple_type>::kind;
} // namespace

// The handlers' results have no common type; each converts to R.
TEST(ExplicitResult, VariantResultWithoutCommonType) {
  const auto cases = on<Transition>(val<Action::start> >> Started{7},
                                    _ >> Stopped{});

  Action     start = Action::start;
  Transition first = match(start) | cases;
  ASSERT_TRUE(std::holds_alternative<Started>(first));
  EXPECT_EQ(std::get<Started>(first).id, 7);

  Action stop = Action::stop;
  EXPECT_TRUE(std::holds_alternative<Stopped>(match(stop) | cases));
  static_assert(
      std::is_same_v<decltype(match(stop) | cases), Transition>);
}

TEST(ExplicitResult, ConvertsEachHandlerResult) {
  auto cases = on<long>(
      lit(1) >> [] { return 1; },
      lit(2) >> short{2},
      $ >> [](int x) { return x * 10; });

  int one   = 1;
  int two   = 2;
  int three = 3;
  static_assert(std::is_same_v<decltype(match(one) | cases), long>);
  EXPECT_EQ(match(one) | cases, 1L);
  EXPECT_EQ(match(two) | cases, 2L);
  EXPECT_EQ(match(three) | cases, 30L);
}

// Unselected handlers are type-checked but never called.
TEST(ExplicitResult, CallsOnlyTheSelectedHandler) {
  int  first  = 0;
  int  second = 0;
  auto cases  = on<std::string>(
      lit(1) >>
          [&] {
            ++first;
            return "one";
          },
      _ >>
          [&] {
            ++second;
            return std::string("other");
          });

  int one = 1;
  EXPECT_EQ(match(one) | cases, "one");
  EXPECT_EQ(first, 1);
  EXPECT_EQ(second, 0);
}

// on<void> discards results, even when they have no common type.
TEST(ExplicitResult, VoidDiscardsResults) {
  int calls = 0;
  int value = 2;
  match(value)
      | on<void>(
          lit(1) >>
              [&] {
                ++calls;
                return std::string("one");
              },
          lit(2) >>
              [&] {
                ++calls;
                return 2;
              },
          _ >> [] {});
  EXPECT_EQ(calls, 1);
  static_assert(
      std::is_void_v<decltype(match(value) | on<void>(_ >> 1))>);
}

TEST(ExplicitResult, ReferenceResultBindsToHandlerResult) {
  Derived derived;
  Base    base;
  auto    cases = on<const Base &>(
      lit(1) >> [&]() -> Derived    &{ return derived; },
      _ >> [&]() -> const Base    &{ return base; });

  int         one    = 1;
  int         two    = 2;
  const Base &chosen = match(one) | cases;
  EXPECT_EQ(&chosen, &derived);
  EXPECT_EQ(chosen.id(), 2);
  EXPECT_EQ(&(match(two) | cases), &base);
  static_assert(
      std::is_same_v<decltype(match(one) | cases), const Base &>);
}

TEST(ExplicitResult, OwnedMoveOnlySubject) {
  auto result = match(std::make_unique<int>(4))
                | on<std::unique_ptr<const int>>(
                    $[_ == nullptr] >>
                        [] { return std::unique_ptr<const int>(); },
                    $ >> [](std::unique_ptr<int> p) { return p; });
  ASSERT_NE(result, nullptr);
  EXPECT_EQ(*result, 4);
}

TEST(ExplicitResult, ExpressionAndValueHandlers) {
  const auto cases = on<std::optional<int>>($[_ > 3] >> _ * 2,
                                            _ >> std::nullopt);

  int five = 5;
  int one  = 1;
  EXPECT_EQ(match(five) | cases, std::optional<int>(10));
  EXPECT_EQ(match(one) | cases, std::nullopt);
}

TEST(ExplicitResult, VariantSubject) {
  using Value      = std::variant<int, std::string>;
  const auto cases = on<std::optional<std::size_t>>(
      $(is<std::string>) >>
          [](const std::string &s) { return s.size(); },
      is<int> >> std::nullopt,
      _ >> std::nullopt);

  Value text = std::string("abc");
  Value num  = 3;
  EXPECT_EQ(match(text) | cases, std::optional<std::size_t>(3));
  EXPECT_EQ(match(num) | cases, std::nullopt);
}

// The result type does not change how cases are dispatched.
TEST(ExplicitResult, KeepsDispatchPlan) {
  using deduced   = decltype(on(
      val<Action::start> >> 1, val<Action::stop> >> 2, _ >> 0));
  using explicit_ = decltype(on<long>(
      val<Action::start> >> 1, val<Action::stop> >> 2, _ >> 0));
  static_assert(
      plan_kind_v<Action, explicit_>
      == dispatch::dispatch_plan_kind::static_literal_dense);
  static_assert(plan_kind_v<Action, explicit_>
                == plan_kind_v<Action, deduced>);

  Action pause = Action::pause;
  EXPECT_EQ(match(pause)
                | on<long>(val<Action::start> >> 1,
                           val<Action::stop> >> 2,
                           _ >> 0),
            0L);
}

TEST(ExplicitResult, StatefulHandlerInStoredPack) {
  auto cases = on<long>(
      lit(0) >> [count = 0]() mutable { return ++count; }, _ >> 0);

  int zero = 0;
  EXPECT_EQ(match(zero) | cases, 1L);
  EXPECT_EQ(match(zero) | cases, 2L);
}

TEST(ExplicitResult, StaticOnCachesExplicitPack) {
  auto run = [](int x) {
    return match(x) | static_on([] {
             return on<double>(lit(1) >> 1, _ >> 2);
           });
  };
  static_assert(std::is_same_v<decltype(run(1)), double>);
  EXPECT_EQ(run(1), 1.0);
  EXPECT_EQ(run(5), 2.0);
}

TEST(ExplicitResult, ConstantEvaluation) {
  constexpr long result = [] {
    int value = 2;
    return match(value) | on<long>(lit(2) >> 9, _ >> 0);
  }();
  static_assert(result == 9L);
  EXPECT_EQ(result, 9L);
}

// on(...) keeps deducing the common type.
TEST(ExplicitResult, DeducedResultUnchanged) {
  int value = 1;
  static_assert(std::is_same_v<decltype(match(value)
                                        | on(lit(1) >> 1, _ >> 2L)),
                               long>);
  const auto statements = on(_ >> [] {});
  static_assert(std::is_void_v<decltype(match(value) | statements)>);
}
