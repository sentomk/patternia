// bench_ownership.cpp — Owning versus borrowed match benchmarks.
//
// Each scenario runs the same case list three ways:
//
//   Borrowed  match(lvalue): the subject is observed by const
//   reference
//             and can use optimized dispatch.
//   Owned     match(rvalue): the context owns the subject and
//   evaluates
//             the cases in source order.
//   Manual    hand-written C++ with the same semantics.
//
// The owned variants copy the workload item into a temporary first,
// so part of the Borrowed/Owned difference for non-trivial subjects
// is that copy. The Consume scenario isolates the opposite
// direction: a subject that the handler takes over, where borrowing
// forces a copy.
//
// These are timing benchmarks only. Ownership semantics are tested
// in tests/tests_move_matching.cpp.

#include <benchmark/benchmark.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "ptn/patternia.hpp"

namespace {

  using namespace ptn;

  // ---- Scenario 1: literal dispatch on a scalar ----

  int literal_borrowed(const int &v) {
    return match(v)
           | on(lit(1) >> 11,
                lit(2) >> 22,
                lit(3) >> 33,
                lit(4) >> 44,
                lit(5) >> 55,
                lit(6) >> 66,
                lit(7) >> 77,
                lit(8) >> 88,
                _ >> 0);
  }

  int literal_owned(const int &v) {
    return match(int(v))
           | on(lit(1) >> 11,
                lit(2) >> 22,
                lit(3) >> 33,
                lit(4) >> 44,
                lit(5) >> 55,
                lit(6) >> 66,
                lit(7) >> 77,
                lit(8) >> 88,
                _ >> 0);
  }

  int literal_manual(const int &v) {
    switch (v) {
    case 1:
      return 11;
    case 2:
      return 22;
    case 3:
      return 33;
    case 4:
      return 44;
    case 5:
      return 55;
    case 6:
      return 66;
    case 7:
      return 77;
    case 8:
      return 88;
    default:
      return 0;
    }
  }

  const std::vector<int> &wl_literal() {
    static const std::vector<int> data = {
        1, 8, 3, 9, 5, 2, 7, 0, 4, 6, 12, 8, 1, 5, 3, 7};
    return data;
  }

  // ---- Scenario 2: variant alternatives with bindings ----

  struct Connect {
    std::uint32_t session;
  };
  struct Message {
    std::uint32_t session;
    std::uint16_t length;
  };
  struct Ack {
    std::uint32_t seq;
  };
  struct Close {
    std::uint16_t reason;
  };
  using Event = std::variant<Connect, Message, Ack, Close>;

  int variant_borrowed(const Event &e) {
    return match(e)
           | on(
               $(is<Connect>) >>
                   [](Connect c) {
                     return static_cast<int>(c.session);
                   },
               $(is<Message>) >>
                   [](Message m) {
                     return static_cast<int>(m.session + m.length);
                   },
               $(is<Ack>) >>
                   [](Ack a) { return static_cast<int>(a.seq); },
               _ >> -1);
  }

  int variant_owned(const Event &e) {
    return match(Event(e))
           | on(
               $(is<Connect>) >>
                   [](Connect c) {
                     return static_cast<int>(c.session);
                   },
               $(is<Message>) >>
                   [](Message m) {
                     return static_cast<int>(m.session + m.length);
                   },
               $(is<Ack>) >>
                   [](Ack a) { return static_cast<int>(a.seq); },
               _ >> -1);
  }

  int variant_manual(const Event &e) {
    struct Visitor {
      int operator()(const Connect &c) const {
        return static_cast<int>(c.session);
      }
      int operator()(const Message &m) const {
        return static_cast<int>(m.session + m.length);
      }
      int operator()(const Ack &a) const {
        return static_cast<int>(a.seq);
      }
      int operator()(const Close &) const {
        return -1;
      }
    };
    return std::visit(Visitor{}, e);
  }

  const std::vector<Event> &wl_variant() {
    static const std::vector<Event> data = {
        Connect{1},
        Message{1, 64},
        Ack{7},
        Close{2},
        Message{2, 12},
        Ack{8},
        Connect{2},
        Message{1, 900},
        Close{0},
        Ack{9},
        Message{2, 3},
        Connect{3},
    };
    return data;
  }

  // ---- Scenario 3: guards rejected before a later case wins ----

  int guards_borrowed(const int &v) {
    return match(v)
           | on($[_ > 1000] >> _ + 4,
                $[_ > 100] >> _ + 3,
                $[_ > 10] >> _ + 2,
                $[_ > 1] >> _ + 1,
                $ >> _ * 2);
  }

  int guards_owned(const int &v) {
    return match(int(v))
           | on($[_ > 1000] >> _ + 4,
                $[_ > 100] >> _ + 3,
                $[_ > 10] >> _ + 2,
                $[_ > 1] >> _ + 1,
                $ >> _ * 2);
  }

  int guards_manual(const int &v) {
    if (v > 1000) {
      return v + 4;
    }
    if (v > 100) {
      return v + 3;
    }
    if (v > 10) {
      return v + 2;
    }
    if (v > 1) {
      return v + 1;
    }
    return v * 2;
  }

  // Most items fall through several guards; the last case wins
  // often.
  const std::vector<int> &wl_guards() {
    static const std::vector<int> data = {
        0, 5, 1, 50, 0, 1, 500, 1, 0, 5000, 1, 0, 7, 1, 0, 60};
    return data;
  }

  // ---- Scenario 4: structural bindings with a guard ----

  struct Point {
    int x;
    int y;
  };

  int structural_borrowed(const Point &p) {
    constexpr auto pt = has<&Point::x, &Point::y>;
    auto [x, y]       = pt;
    return match(p)
           | on($(pt)[x > 0 && y > 0] >> x + y,
                $(pt)[x > 0] >> x - y,
                $(pt) >> x * x + y * y,
                _ >> 0);
  }

  int structural_owned(const Point &p) {
    constexpr auto pt = has<&Point::x, &Point::y>;
    auto [x, y]       = pt;
    return match(Point(p))
           | on($(pt)[x > 0 && y > 0] >> x + y,
                $(pt)[x > 0] >> x - y,
                $(pt) >> x * x + y * y,
                _ >> 0);
  }

  int structural_manual(const Point &p) {
    if (p.x > 0 && p.y > 0) {
      return p.x + p.y;
    }
    if (p.x > 0) {
      return p.x - p.y;
    }
    return p.x * p.x + p.y * p.y;
  }

  const std::vector<Point> &wl_structural() {
    static const std::vector<Point> data = {
        {1, 2},
        {3, -4},
        {-5, 6},
        {0, 0},
        {7, 8},
        {9, -1},
        {-2, -3},
        {4, 0},
        {0, 5},
        {6, 6},
        {-7, 1},
        {8, -8},
    };
    return data;
  }

  template <typename T, typename F>
  void
  run_wl(benchmark::State &state, const std::vector<T> &wl, F fn) {
    std::size_t idx = 0;
    int         acc = 0;
    for (auto _ : state) {
      const T *item = &wl[idx];
      benchmark::DoNotOptimize(item);
      acc += fn(*item);
      benchmark::DoNotOptimize(acc);
      benchmark::ClobberMemory();
      ++idx;
      if (idx == wl.size()) {
        idx = 0;
      }
    }
    benchmark::DoNotOptimize(acc);
    state.SetItemsProcessed(state.iterations());
  }

  // ---- Scenario 5: the handler takes over a heap-backed subject
  // ----
  //
  // A string longer than the small-string buffer is passed through a
  // match and stored back. Owned moves it in and out; Borrowed has
  // to copy, which allocates.

  std::string make_payload() {
    return std::string(256, 'x');
  }

  template <typename F>
  void run_consume(benchmark::State &state, F step) {
    std::string slot = make_payload();
    for (auto _ : state) {
      benchmark::DoNotOptimize(slot);
      step(slot);
      benchmark::ClobberMemory();
    }
    benchmark::DoNotOptimize(slot);
    state.SetItemsProcessed(state.iterations());
  }

  void consume_borrowed(std::string &slot) {
    std::string next = match(slot)
                       | on($ >> [](const std::string &s) {
                           return std::string(s);
                         });
    slot = std::move(next);
  }

  void consume_owned(std::string &slot) {
    slot = match(std::move(slot))
           | on($ >> [](std::string s) { return s; });
  }

  // Same ownership steps as the owned match: move the subject out,
  // pass it to a by-value handler, and store the returned value.
  void consume_manual(std::string &slot) {
    const auto  take  = [](std::string s) { return s; };
    std::string owned = std::move(slot);
    slot              = take(std::move(owned));
  }

  // ---- Scenario 6: rejected guards before a consuming fallback
  // ----
  //
  // Two guards observe the string and fail; the fallback then takes
  // it over. Guards must not consume the owned value.

  void consume_guarded_borrowed(std::string &slot) {
    std::string next = match(slot)
                       | on(
                           $[([](const std::string &s) {
                             return s.empty();
                           })] >>
                               [](const std::string &) {
                                 return std::string();
                               },
                           $[([](const std::string &s) {
                             return s.size() < 8;
                           })] >>
                               [](const std::string &s) {
                                 return std::string(s);
                               },
                           $ >>
                               [](const std::string &s) {
                                 return std::string(s);
                               });
    slot = std::move(next);
  }

  void consume_guarded_owned(std::string &slot) {
    slot = match(std::move(slot))
           | on(
               $[([](const std::string &s) { return s.empty(); })] >>
                   [](std::string) { return std::string(); },
               $[([](const std::string &s) { return s.size() < 8; })]
                   >> [](std::string s) { return s; },
               $ >> [](std::string s) { return s; });
  }

  void consume_guarded_manual(std::string &slot) {
    const auto  take  = [](std::string s) { return s; };
    std::string owned = std::move(slot);
    if (owned.empty()) {
      slot = std::string();
    }
    else if (owned.size() < 8) {
      slot = take(std::move(owned));
    }
    else {
      slot = take(std::move(owned));
    }
  }

#define PTN_OWNERSHIP_BENCH(Name)                                   \
  BENCHMARK(BM_##Name)                                              \
      ->Unit(benchmark::kNanosecond)                                \
      ->MinTime(0.2)                                                \
      ->Repetitions(10)                                             \
      ->ReportAggregatesOnly(true)

#define DEF_BENCH(Name, Wl, Fn)                                     \
  static void BM_##Name(benchmark::State &state) {                  \
    run_wl(state, Wl(), Fn);                                        \
  }                                                                 \
  PTN_OWNERSHIP_BENCH(Name)

#define DEF_CONSUME_BENCH(Name, Fn)                                 \
  static void BM_##Name(benchmark::State &state) {                  \
    run_consume(state, Fn);                                         \
  }                                                                 \
  PTN_OWNERSHIP_BENCH(Name)

  DEF_BENCH(Borrowed_Literal, wl_literal, literal_borrowed);
  DEF_BENCH(Owned_Literal, wl_literal, literal_owned);
  DEF_BENCH(Manual_Literal, wl_literal, literal_manual);

  DEF_BENCH(Borrowed_Variant, wl_variant, variant_borrowed);
  DEF_BENCH(Owned_Variant, wl_variant, variant_owned);
  DEF_BENCH(Manual_Variant, wl_variant, variant_manual);

  DEF_BENCH(Borrowed_RejectedGuards, wl_guards, guards_borrowed);
  DEF_BENCH(Owned_RejectedGuards, wl_guards, guards_owned);
  DEF_BENCH(Manual_RejectedGuards, wl_guards, guards_manual);

  DEF_BENCH(Borrowed_Structural, wl_structural, structural_borrowed);
  DEF_BENCH(Owned_Structural, wl_structural, structural_owned);
  DEF_BENCH(Manual_Structural, wl_structural, structural_manual);

  DEF_CONSUME_BENCH(Borrowed_Consume, consume_borrowed);
  DEF_CONSUME_BENCH(Owned_Consume, consume_owned);
  DEF_CONSUME_BENCH(Manual_Consume, consume_manual);

  DEF_CONSUME_BENCH(Borrowed_ConsumeGuarded,
                    consume_guarded_borrowed);
  DEF_CONSUME_BENCH(Owned_ConsumeGuarded, consume_guarded_owned);
  DEF_CONSUME_BENCH(Manual_ConsumeGuarded, consume_guarded_manual);

} // namespace
