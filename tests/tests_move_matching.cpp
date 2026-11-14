#include <gtest/gtest.h>

#include <ptn/patternia.hpp>

#include <memory>
#include <string>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <variant>

using namespace ptn;

namespace {
  struct Counts {
    int copies = 0;
    int moves  = 0;
    int live   = 0;
  };
  struct Tracked {
    Counts *counts;
    int     value;
    explicit Tracked(Counts &c, int v = 7) : counts(&c), value(v) {
      ++c.live;
    }
    Tracked(const Tracked &other)
        : counts(other.counts), value(other.value) {
      ++counts->copies;
      ++counts->live;
    }
    Tracked(Tracked &&other) noexcept
        : counts(other.counts), value(other.value) {
      ++counts->moves;
      ++counts->live;
      other.value = -1;
    }
    ~Tracked() {
      --counts->live;
    }
  };
  struct Record {
    std::unique_ptr<int> first;
    std::unique_ptr<int> second;
    const int            tag = 3;
  };
  PTN_BIND(Record, first, second, tag);

  struct RvalueHandler {
    std::unique_ptr<int> value;
    explicit RvalueHandler(int v) : value(std::make_unique<int>(v)) {
    }
    RvalueHandler(RvalueHandler &&)      = default;
    RvalueHandler(const RvalueHandler &) = delete;
    int operator()(int n) && {
      return n + *value;
    }
  };

  // Materializes a move-only binding. Guards and handlers must see
  // the same object, even through a nested variant type pattern.
  struct Materialize : pat::base::pattern_base<Materialize>,
                       pat::base::binding_pattern_base<Materialize> {
    int *binds;
    explicit Materialize(int &count) : binds(&count) {
    }
    bool match(const int &) const {
      return true;
    }
    auto bind(const int &value) const {
      ++*binds;
      return std::make_tuple(std::make_unique<int>(value));
    }
  };
} // namespace

namespace ptn::pat::base {
  template <typename S>
  struct binding_args<::Materialize, S> {
    using type = std::tuple<std::unique_ptr<int>>;
  };
} // namespace ptn::pat::base

TEST(MoveMatching, BorrowedSubjectKeepsConstBindingsAndDoesNotCopy) {
  Counts     counts;
  Tracked    value(counts);
  const auto result = match(value) | on($ >> [&](auto &&bound) {
                        static_assert(
                            std::is_same_v<decltype(bound),
                                           const Tracked &>);
                        EXPECT_EQ(&bound, &value);
                        return bound.value;
                      });
  EXPECT_EQ(result, 7);
  EXPECT_EQ(counts.copies, 0);
  EXPECT_EQ(counts.moves, 0);
}

TEST(MoveMatching, ContextOwnsTemporaryUntilDelayedEvaluationEnds) {
  Counts counts;
  {
    auto context = match(Tracked(counts, 11));
    EXPECT_EQ(counts.live, 1);
    EXPECT_EQ(counts.copies, 0);
    EXPECT_EQ(counts.moves, 1);
    EXPECT_EQ(std::move(context)
                  | on($ >>
                       [](Tracked owned) {
                         return std::exchange(owned.value, -2);
                       }),
              11);
    EXPECT_EQ(counts.moves, 2);
    EXPECT_EQ(counts.live, 1);
  }
  EXPECT_EQ(counts.live, 0);
}

TEST(MoveMatching, ExplicitMoveConsumesSourceAtContextConstruction) {
  auto value   = std::make_unique<int>(17);
  auto context = match(std::move(value));
  EXPECT_EQ(value, nullptr);
  auto result = std::move(context)
                | on($ >> [](std::unique_ptr<int> owned) {
                    return owned;
                  });
  ASSERT_NE(result, nullptr);
  EXPECT_EQ(*result, 17);
}

TEST(MoveMatching, RejectedGuardLeavesOwnedSubjectForFallback) {
  int  guards            = 0;
  int  rejected_handlers = 0;
  auto result            = match(std::make_unique<int>(9))
                | on(
                    $[([&](const std::unique_ptr<int> &value) {
                      ++guards;
                      EXPECT_EQ(*value, 9);
                      return false;
                    })] >>
                        [&](std::unique_ptr<int>) {
                          ++rejected_handlers;
                          return std::unique_ptr<int>{};
                        },
                    $() >>
                        [](std::unique_ptr<int> value) {
                          return value;
                        });
  EXPECT_EQ(guards, 1);
  EXPECT_EQ(rejected_handlers, 0);
  ASSERT_NE(result, nullptr);
  EXPECT_EQ(*result, 9);
}

TEST(MoveMatching, SelectedGuardOnlyObservesBeforeConsumption) {
  Counts counts;
  int    guards = 0;
  EXPECT_EQ(match(Tracked(counts))
                | on(
                    $[([&](const Tracked &value) {
                      ++guards;
                      EXPECT_EQ(counts.moves, 1);
                      return value.value == 7;
                    })] >>
                        [](Tracked value) {
                          return std::exchange(value.value, -2);
                        },
                    _ >> -1),
            7);
  EXPECT_EQ(guards, 1);
  EXPECT_EQ(counts.copies, 0);
  EXPECT_EQ(counts.moves, 2);
  EXPECT_EQ(counts.live, 0);
}

TEST(MoveMatching, ConstRvaluePreservesConstness) {
  Counts        counts;
  const Tracked value(counts);
  EXPECT_EQ(
      match(static_cast<decltype(value) &&>(value))
          | on($ >>
               [](auto &&bound) {
                 static_assert(std::is_same_v<decltype(bound),
                                              const Tracked &&>);
                 return bound.value;
               }),
      7);
  EXPECT_EQ(counts.copies, 1);
  EXPECT_EQ(counts.moves, 0);
  EXPECT_EQ(value.value, 7);
}

TEST(MoveMatching, StructuralMembersMoveOnlyAfterGuardWins) {
  Record subject;
  subject.first  = std::make_unique<int>(2);
  subject.second = std::make_unique<int>(5);
  int  guards    = 0;
  auto result    = match(std::move(subject))
                | on(
                    $(has<&Record::first, _ign, &Record::second>)[(
                        [&](const auto &a, const auto &b) {
                          ++guards;
                          return *a + *b == 7;
                        })]
                        >>
                        [](std::unique_ptr<int> a,
                           std::unique_ptr<int> b) {
                          return *a * *b;
                        },
                    _ >> -1);
  EXPECT_EQ(result, 10);
  EXPECT_EQ(guards, 1);
  // Moving a unique_ptr is specified to leave the source empty.
  // NOLINTNEXTLINE(bugprone-use-after-move)
  EXPECT_EQ(subject.first, nullptr);
  EXPECT_EQ(subject.second, nullptr);
}

TEST(MoveMatching, StructuralConstMemberStaysConst) {
  EXPECT_EQ(
      match(Record{})
          | on(
              $(has<&Record::tag>) >>
                  [](auto &&value) {
                    static_assert(std::is_same_v<decltype(value),
                                                 const int &&>);
                    return value;
                  },
              _ >> -1),
      3);
}

TEST(MoveMatching, VariantAlternativesMoveByTypeAndIndex) {
  using V      = std::variant<int, std::unique_ptr<int>>;
  auto by_type = match(V(std::make_unique<int>(12)))
                 | on(
                     $(is<std::unique_ptr<int>>) >>
                         [](std::unique_ptr<int> p) { return p; },
                     _ >> [] { return std::unique_ptr<int>{}; });
  auto by_index = match(V(std::make_unique<int>(13)))
                  | on(
                      $(alt<1>) >>
                          [](std::unique_ptr<int> p) { return p; },
                      _ >> [] { return std::unique_ptr<int>{}; });
  ASSERT_NE(by_type, nullptr);
  ASSERT_NE(by_index, nullptr);
  EXPECT_EQ(*by_type, 12);
  EXPECT_EQ(*by_index, 13);
}

TEST(MoveMatching, NestedVariantBindingRejectsWithoutConsuming) {
  using V     = std::variant<int, std::unique_ptr<int>>;
  auto result = match(V(std::make_unique<int>(14)))
                | on(
                    is<std::unique_ptr<int>>(
                        $[([](const auto &) { return false; })])
                        >> [](std::unique_ptr<int>) { return -1; },
                    alt<1>($) >>
                        [](std::unique_ptr<int> p) { return *p; },
                    _ >> 0);
  EXPECT_EQ(result, 14);
  EXPECT_EQ(
      match(V(6))
          | on(is<std::unique_ptr<int>> >> -1, alt<0> >> 6, _ >> 0),
      6);
}

TEST(MoveMatching, ConstVariantAndNestedMembersStayConst) {
  const std::variant<int, std::string> value(std::string("abc"));
  EXPECT_EQ(match(static_cast<decltype(value) &&>(value))
                | on(
                    is<std::string>($) >>
                        [](auto &&s) {
                          static_assert(
                              std::is_same_v<decltype(s),
                                             const std::string &&>);
                          return s.size();
                        },
                    _ >> std::size_t{0}),
            3u);
  using V = std::variant<int, Record>;
  V nested(std::in_place_index<1>);
  std::get<1>(nested).first = std::make_unique<int>(4);
  EXPECT_EQ(match(std::move(nested))
                | on(
                    alt<1>($(has<&Record::first>)) >>
                        [](std::unique_ptr<int> p) { return *p; },
                    _ >> 0),
            4);
  EXPECT_EQ(std::get<1>(nested).first, nullptr);
}

TEST(MoveMatching, CombinatorsObserveAndShortCircuit) {
  int  first = 0, second = 0, skipped = 0;
  auto reject = $[([&](const auto &p) {
    ++first;
    EXPECT_EQ(*p, 5);
    return false;
  })];
  auto accept = $[([&](const auto &p) {
    ++second;
    EXPECT_EQ(*p, 5);
    return true;
  })];
  auto never  = pred([&](const auto &) {
    ++skipped;
    return true;
  });
  EXPECT_EQ(match(std::make_unique<int>(5))
                | on(
                    all(reject, never) >> -1,
                    $(any(reject, accept, never)) >>
                        [](std::unique_ptr<int> p) { return *p; },
                    _ >> 0),
            5);
  EXPECT_EQ(first, 2);
  EXPECT_EQ(second, 1);
  EXPECT_EQ(skipped, 0);
}

TEST(MoveMatching, NegationAndAllDoNotConsumeBindings) {
  auto result = match(std::make_unique<int>(8))
                | on(
                    $(all(
                        neg($[(
                            [](const auto &p) { return *p < 0; })]),
                        pred([](const auto &p) { return *p == 8; })))
                        >> [](std::unique_ptr<int> p) { return *p; },
                    _ >> 0);
  EXPECT_EQ(result, 8);
}

TEST(MoveMatching, MaterializedBindingLivesThroughGuardAndHandler) {
  int                         binds   = 0;
  const std::unique_ptr<int> *address = nullptr;
  auto pattern = Materialize(binds)[([&](const auto &value) {
    address = &value;
    EXPECT_EQ(*value, 21);
    return true;
  })];
  EXPECT_EQ(match(21)
                | on(
                    pattern >>
                        [&](std::unique_ptr<int> &&value) {
                          EXPECT_EQ(&value, address);
                          address = nullptr;
                          return *value;
                        },
                    _ >> 0),
            21);
  EXPECT_EQ(binds, 1);
}

TEST(MoveMatching, NestedGuardUsesSameMaterializedBinding) {
  int                         binds   = 0;
  const std::unique_ptr<int> *address = nullptr;
  auto pattern = Materialize(binds)[([&](const auto &value) {
    address = &value;
    EXPECT_EQ(*value, 23);
    return true;
  })];
  using V      = std::variant<int, std::string>;
  EXPECT_EQ(match(V(23))
                | on(
                    is<int>(pattern) >>
                        [&](std::unique_ptr<int> &&value) {
                          EXPECT_EQ(&value, address);
                          address = nullptr;
                          return *value;
                        },
                    _ >> 0),
            23);
  EXPECT_EQ(binds, 1);
}

TEST(MoveMatching,
     RejectedMaterializedBindingLeavesOriginalSubject) {
  int  binds   = 0;
  auto pattern = Materialize(
      binds)[([](const auto &) { return false; })];
  EXPECT_EQ(
      match(25)
          | on(
              pattern >> [](std::unique_ptr<int>) { return -1; },
              $ >> [](int value) { return value; }),
      25);
  EXPECT_EQ(binds, 1);
}

TEST(MoveMatching, MoveOnlyMutableHandlerRunsOnce) {
  auto valid = on($ >>
                  [p = std::make_unique<int>(4)](int value) mutable {
                    int result = value + *p;
                    p.reset();
                    return result;
                  });
  EXPECT_EQ(match(3) | std::move(valid), 7);
}

TEST(MoveMatching, OneShotHandlerUsesRvalueCallOperator) {
  EXPECT_EQ(match(3) | on($ >> RvalueHandler(4)), 7);
}

TEST(MoveMatching, ReusableCasePackDoesNotMoveItsHandler) {
  auto cases = on($ >> [p = std::make_unique<int>(4)](int value) {
    return value + *p;
  });
  EXPECT_EQ(match(3) | cases, 7);
  EXPECT_EQ(match(5) | cases, 9);
}

TEST(MoveMatching, OneShotMoveOnlyValueHandlerReturnsOwnedValue) {
  auto result = match(3)
                | on(lit(3) >> std::make_unique<int>(31),
                     _ >> std::unique_ptr<int>{});
  ASSERT_NE(result, nullptr);
  EXPECT_EQ(*result, 31);
}

TEST(MoveMatching, BareExpressionTransfersSelectedBinding) {
  auto result = match(std::make_unique<int>(37)) | on($ >> _);
  ASSERT_NE(result, nullptr);
  EXPECT_EQ(*result, 37);
  Record subject;
  subject.first = std::make_unique<int>(41);
  auto member   = match(std::move(subject))
                | on(
                    $(has<&Record::first>) >> first,
                    _ >> [] { return std::unique_ptr<int>{}; });
  ASSERT_NE(member, nullptr);
  EXPECT_EQ(*member, 41);
  // Moving a unique_ptr is specified to leave the source empty.
  // NOLINTNEXTLINE(bugprone-use-after-move)
  EXPECT_EQ(subject.first, nullptr);
}

TEST(MoveMatching, RepeatedExpressionReadsAndMaterializesResult) {
  EXPECT_EQ(match(4) | on($ >> _ * _ + _), 20);
  auto result = match(std::string("a")) | on($ >> _ + _);
  EXPECT_EQ(result, "aa");
}

TEST(MoveMatching,
     HandlerReferenceResultIsMaterializedBeforeDestruction) {
  auto result = match(std::make_unique<int>(43))
                | on($ >> [](std::unique_ptr<int> &&p)
                         -> std::unique_ptr<int> && {
                    return std::move(p);
                  });
  static_assert(
      std::is_same_v<decltype(result), std::unique_ptr<int>>);
  ASSERT_NE(result, nullptr);
  EXPECT_EQ(*result, 43);
}

TEST(MoveMatching, VoidAndNullaryHandlersRemainSupported) {
  int calls = 0;
  match(3)
      | on(lit(2) >> [&] { calls += 100; }, $ >> [&] { ++calls; });
  match(Record{})
      | on($(has<>) >> [&] { ++calls; }, _ >> [&] { calls += 100; });
  EXPECT_EQ(calls, 2);
}

TEST(MoveMatching,
     NestedAsBindingKeepsSubjectAndMaterializedValueAlive) {
  int binds = 0;
  EXPECT_EQ(match(47)
                | on(
                    $(Materialize(binds)) >>
                        [](int                  original,
                           std::unique_ptr<int> materialized) {
                          return original + *materialized;
                        },
                    _ >> 0),
            94);
  EXPECT_EQ(binds, 1);
}

TEST(MoveMatching,
     QualifierNormalizedTypePatternMatchesOwnedVariant) {
  EXPECT_EQ(match(std::variant<int, double>(5))
                | on(is<const int> >> 1, _ >> 0),
            1);
}

TEST(MoveMatching,
     BorrowedConstSubjectAndReusableValueHandlerRemainValid) {
  const int value = 3;
  auto      cases = on($ >> 7);
  EXPECT_EQ(match(value) | cases, 7);
  EXPECT_EQ(match(5) | cases, 7);
}

TEST(MoveMatching, StoredContextCanBeMovedWithoutDanglingBindings) {
  auto first_context  = match(std::make_unique<int>(53));
  auto second_context = std::move(first_context);
  EXPECT_EQ(std::move(second_context)
                | on($ >> [](std::unique_ptr<int> p) { return *p; }),
            53);
}

TEST(MoveMatching, GuardExceptionReleasesOwnedSubject) {
  Counts counts;
  EXPECT_THROW((match(Tracked(counts))
                | on(
                    $[([](const Tracked &) -> bool {
                      throw std::runtime_error("test");
                    })] >>
                        [](const Tracked &) {},
                    _ >> [] {})),
               std::runtime_error);
  EXPECT_EQ(counts.copies, 0);
  EXPECT_EQ(counts.moves, 1);
  EXPECT_EQ(counts.live, 0);
}

namespace {
  struct ThrowingMove {
    ThrowingMove() = default;
    // Deliberately throw to create a valueless variant for the test.
    // NOLINTNEXTLINE(bugprone-unsafe-to-allow-exceptions)
    ThrowingMove(ThrowingMove &&) noexcept(false) {
      throw std::runtime_error("test");
    }
    ThrowingMove &operator=(ThrowingMove &&) = default;
  };
} // namespace

TEST(MoveMatching, ValuelessVariantReachesFallback) {
  using V = std::variant<std::monostate, ThrowingMove>;
  V source(std::in_place_index<1>);
  V subject;
  EXPECT_THROW(subject = std::move(source), std::runtime_error);
  ASSERT_TRUE(subject.valueless_by_exception());
  EXPECT_EQ(match(std::move(subject))
                | on(is<ThrowingMove> >> 1, alt<0> >> 2, _ >> 3),
            3);
}

TEST(MoveMatching,
     AsBindingPreservesNestedMaterializedObjectIdentity) {
  int                         binds   = 0;
  const std::unique_ptr<int> *address = nullptr;
  auto inner = Materialize(binds)[([&](const auto &p) {
    address = &p;
    return true;
  })];
  EXPECT_EQ(match(59)
                | on(
                    $(inner) >>
                        [&](int n, std::unique_ptr<int> &&p) {
                          EXPECT_EQ(&p, address);
                          address = nullptr;
                          return n + *p;
                        },
                    _ >> 0),
            118);
  EXPECT_EQ(binds, 1);
}

TEST(MoveMatching, BorrowedSubjectAcceptsMoveOnlyMutableHandler) {
  int  subject = 3;
  auto cases   = on($ >>
                  [p = std::make_unique<int>(4)](int value) mutable {
                    const int result = value + *p;
                    p.reset();
                    return result;
                  });
  EXPECT_EQ(match(subject) | std::move(cases), 7);
  EXPECT_EQ(subject, 3);
}

TEST(MoveMatching, BorrowedSubjectCanConsumeMaterializedBinding) {
  int  binds   = 0;
  int  subject = 61;
  auto pattern = Materialize(binds);
  EXPECT_EQ(
      match(subject)
          | on(
              pattern >> [](std::unique_ptr<int> p) { return *p; },
              _ >> 0),
      61);
  EXPECT_EQ(binds, 1);
  EXPECT_EQ(subject, 61);
}

TEST(MoveMatching,
     BorrowedGuardCanConsumeSelectedMaterializedBinding) {
  int  binds   = 0;
  int  subject = 67;
  auto pattern = Materialize(
      binds)[([](const auto &p) { return *p == 67; })];
  EXPECT_EQ(
      match(subject)
          | on(
              pattern >> [](std::unique_ptr<int> p) { return *p; },
              _ >> 0),
      67);
  EXPECT_EQ(binds, 1);
  EXPECT_EQ(subject, 67);
}

TEST(MoveMatching, BorrowedMaterializedBindingSupportsVoidHandler) {
  int                  binds   = 0;
  int                  subject = 71;
  std::unique_ptr<int> result;
  match(subject)
      | on(
          Materialize(binds) >>
              [&](std::unique_ptr<int> p) { result = std::move(p); },
          _ >> [] {});
  EXPECT_EQ(binds, 1);
  ASSERT_NE(result, nullptr);
  EXPECT_EQ(*result, 71);
}

TEST(MoveMatching, MoveOnlyMutableGuardStillObservesConstBindings) {
  EXPECT_EQ(match(std::make_unique<int>(73))
                | on(
                    $[([state = std::make_unique<int>(73)](
                           const auto &p) mutable {
                      bool accepted = *state == *p;
                      state.reset();
                      return accepted;
                    })] >>
                        [](std::unique_ptr<int> p) { return *p; },
                    _ >> 0),
            73);
}
