#include <benchmark/benchmark.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include "ptn/patternia.hpp"

namespace {

  using V = std::variant<int, std::string>;

  struct Packet {
    std::uint8_t              type;
    std::uint16_t             length;
    std::uint8_t              flags;
    std::vector<std::uint8_t> payload;
  };

  constexpr std::uint8_t FLAG_VALID = 0x01;

  static int patternia_variant_route(const V &v) {
    using namespace ptn;
    using ptn::pat::type::is;

    return match(v)
        .when(is<int>() >> 1)
        .when(is<std::string>() >> 2)
        .when(__ >> 0)
        .end();
  }

  static int std_visit_variant_route(const V &v) {
    return std::visit(
        [](const auto &x) -> int {
          using T = std::decay_t<decltype(x)>;
          if constexpr (std::is_same_v<T, int>) {
            return 1;
          }
          else if constexpr (std::is_same_v<T, std::string>) {
            return 2;
          }
          else {
            return 0;
          }
        },
        v);
  }

  static int switch_index_variant_route(const V &v) {
    switch (v.index()) {
    case 0:
      return 1;
    case 1:
      return 2;
    default:
      return 0;
    }
  }

  static int patternia_packet_route(const Packet &pkt) {
    using namespace ptn;

    auto is_ping_packet = [](std::uint8_t type, std::uint16_t length) {
      return type == 0x01 && length == 0;
    };

    auto is_valid_data_packet =
        [&pkt](std::uint8_t type, std::uint16_t length, std::uint8_t flags) {
          return type == 0x02 && length == pkt.payload.size()
                 && (flags & FLAG_VALID);
        };

    auto is_error_packet = [&pkt](std::uint8_t type) {
      return type == 0xFF && !pkt.payload.empty();
    };

    return match(pkt)
        .when(bind(has<&Packet::type, &Packet::length>())[is_ping_packet] >> 1)
        .when(bind(has<&Packet::type, &Packet::length, &Packet::flags>())
                  [is_valid_data_packet]
              >> 2)
        .when(bind(has<&Packet::type>())[is_error_packet] >> 3)
        .otherwise(0);
  }

  static int switch_packet_route(const Packet &pkt) {
    switch (pkt.type) {
    case 0x01:
      if (pkt.length == 0) {
        return 1;
      }
      break;
    case 0x02:
      if (pkt.length == pkt.payload.size() && (pkt.flags & FLAG_VALID)) {
        return 2;
      }
      break;
    case 0xFF:
      if (!pkt.payload.empty()) {
        return 3;
      }
      break;
    default:
      break;
    }
    return 0;
  }

  static const std::vector<V> &variant_workload() {
    static const std::vector<V> data = {
        1,  std::string("a"),     2,  std::string("bb"),
        3,  std::string("ccc"),   4,  std::string("dddd"),
        5,  std::string("eeeee"), 6,  std::string("ffffff"),
        7,  std::string("gg"),    8,  std::string("hhh"),
        9,  std::string("iiii"),  10, std::string("jjjjj"),
        11, std::string("k"),     12, std::string("ll"),
        13, std::string("mmm"),   14, std::string("nnnn"),
        15, std::string("ooooo"), 16, std::string("pppppp"),
        17, std::string("qq"),    18, std::string("rrr"),
        19, std::string("ssss"),  20, std::string("ttttt"),
        21, std::string("u"),     22, std::string("vv"),
        23, std::string("www"),   24, std::string("xxxx"),
        25, std::string("yyyyy"), 26, std::string("zzzzzz"),
        27, std::string("ab"),    28, std::string("abc"),
        29, std::string("abcd"),  30, std::string("abcde"),
        31, std::string("f"),     32, std::string("gh"),
    };
    return data;
  }

  static const std::vector<Packet> &packet_workload() {
    static const std::vector<Packet> data = {
        {0x01, 0, 0, {}},
        {0x02, 3, FLAG_VALID, {1, 2, 3}},
        {0xFF, 1, 0, {42}},
        {0x02, 10, 0, {1}},
        {0x01, 1, 0, {}},
        {0x02, 4, FLAG_VALID, {9, 8, 7, 6}},
        {0xFF, 0, 0, {}},
        {0x02, 2, FLAG_VALID, {5, 5}},
    };
    return data;
  }

  template <typename T, typename F>
  static void
  run_workload(benchmark::State &state, const std::vector<T> &workload, F fn) {
    std::size_t idx = 0;
    int         acc = 0;

    for (auto _ : state) {
      const T *item = &workload[idx];
      benchmark::DoNotOptimize(item);
      acc += fn(*item);
      benchmark::DoNotOptimize(acc);
      benchmark::ClobberMemory();

      ++idx;
      if (idx == workload.size()) {
        idx = 0;
      }
    }

    benchmark::DoNotOptimize(acc);
    state.SetItemsProcessed(state.iterations());
  }

  static void BM_Patternia_VariantMixed(benchmark::State &state) {
    run_workload(state, variant_workload(), patternia_variant_route);
  }

  static void BM_StdVisit_VariantMixed(benchmark::State &state) {
    run_workload(state, variant_workload(), std_visit_variant_route);
  }

  static void BM_SwitchIndex_VariantMixed(benchmark::State &state) {
    run_workload(state, variant_workload(), switch_index_variant_route);
  }

  static void BM_Patternia_PacketMixed(benchmark::State &state) {
    run_workload(state, packet_workload(), patternia_packet_route);
  }

  static void BM_Switch_PacketMixed(benchmark::State &state) {
    run_workload(state, packet_workload(), switch_packet_route);
  }

} // namespace

BENCHMARK(BM_Patternia_VariantMixed);
BENCHMARK(BM_StdVisit_VariantMixed);
BENCHMARK(BM_SwitchIndex_VariantMixed);
BENCHMARK(BM_Patternia_PacketMixed);
BENCHMARK(BM_Switch_PacketMixed);
