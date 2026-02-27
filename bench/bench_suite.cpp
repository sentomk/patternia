#include <benchmark/benchmark.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include "ptn/patternia.hpp"

#define PTN_BENCH_MODE_PIPE_STD   1
#define PTN_BENCH_MODE_PIPE_CHAIN 2

#ifndef PTN_BENCH_MODE
#define PTN_BENCH_MODE PTN_BENCH_MODE_PIPE_STD
#endif

#ifndef PTN_BENCH_ENABLE_SUITE_VARIANT
#define PTN_BENCH_ENABLE_SUITE_VARIANT 1
#endif

#ifndef PTN_BENCH_ENABLE_SUITE_VARIANT_GUARDED
#define PTN_BENCH_ENABLE_SUITE_VARIANT_GUARDED 1
#endif

#ifndef PTN_BENCH_ENABLE_SUITE_VARIANT_FASTPATH
#define PTN_BENCH_ENABLE_SUITE_VARIANT_FASTPATH 1
#endif

#ifndef PTN_BENCH_ENABLE_SUITE_PROTOCOL_ROUTER
#define PTN_BENCH_ENABLE_SUITE_PROTOCOL_ROUTER 1
#endif

#ifndef PTN_BENCH_ENABLE_SUITE_COMMAND_PARSER
#define PTN_BENCH_ENABLE_SUITE_COMMAND_PARSER 1
#endif

#ifndef PTN_BENCH_ENABLE_SUITE_LITERAL_MATCH
#define PTN_BENCH_ENABLE_SUITE_LITERAL_MATCH 1
#endif

#ifndef PTN_BENCH_ENABLE_SUITE_LITERAL_MATCH_128
#define PTN_BENCH_ENABLE_SUITE_LITERAL_MATCH_128 1
#endif

#ifndef PTN_BENCH_ENABLE_SUITE_PACKET
#define PTN_BENCH_ENABLE_SUITE_PACKET 1
#endif

#ifndef PTN_BENCH_ENABLE_SUITE_PACKET_HEAVY_BIND
#define PTN_BENCH_ENABLE_SUITE_PACKET_HEAVY_BIND 1
#endif

namespace {

  using V = std::variant<int, std::string>;

  template <int I>
  struct VariantAltToken {
    int value;
  };

  using VAlt32 = std::variant<VariantAltToken<0>,
                              VariantAltToken<1>,
                              VariantAltToken<2>,
                              VariantAltToken<3>,
                              VariantAltToken<4>,
                              VariantAltToken<5>,
                              VariantAltToken<6>,
                              VariantAltToken<7>,
                              VariantAltToken<8>,
                              VariantAltToken<9>,
                              VariantAltToken<10>,
                              VariantAltToken<11>,
                              VariantAltToken<12>,
                              VariantAltToken<13>,
                              VariantAltToken<14>,
                              VariantAltToken<15>,
                              VariantAltToken<16>,
                              VariantAltToken<17>,
                              VariantAltToken<18>,
                              VariantAltToken<19>,
                              VariantAltToken<20>,
                              VariantAltToken<21>,
                              VariantAltToken<22>,
                              VariantAltToken<23>,
                              VariantAltToken<24>,
                              VariantAltToken<25>,
                              VariantAltToken<26>,
                              VariantAltToken<27>,
                              VariantAltToken<28>,
                              VariantAltToken<29>,
                              VariantAltToken<30>,
                              VariantAltToken<31>>;

  struct Packet {
    std::uint8_t              type;
    std::uint16_t             length;
    std::uint8_t              flags;
    std::vector<std::uint8_t> payload;
  };

  struct ProtoPing {
    std::uint32_t seq;
    bool          urgent;
  };

  struct ProtoData {
    std::uint16_t stream_id;
    std::uint16_t length;
    bool          reliable;
  };

  struct ProtoError {
    std::uint16_t code;
    bool          fatal;
  };

  struct ProtoControl {
    std::uint8_t op;
    std::uint8_t channel;
    bool         ack_required;
  };

  using ProtocolMsg = std::
      variant<ProtoPing, ProtoData, ProtoError, ProtoControl>;

  struct CmdSet {
    std::uint16_t key;
    std::int32_t  value;
    bool          persist;
  };

  struct CmdGet {
    std::uint16_t key;
    bool          allow_stale;
  };

  struct CmdDel {
    std::uint16_t key;
    bool          recursive;
  };

  struct CmdScan {
    std::uint16_t prefix;
    std::uint16_t limit;
  };

  using CommandMsg = std::variant<CmdSet, CmdGet, CmdDel, CmdScan>;

  constexpr std::uint8_t FLAG_VALID = 0x01;

  static std::vector<std::uint8_t> make_payload(std::size_t  size,
                                                std::uint8_t seed) {
    std::vector<std::uint8_t> payload(size);
    for (std::size_t i = 0; i < size; ++i) {
      payload[i] = static_cast<std::uint8_t>((seed + i * 13u)
                                             & 0xFFu);
    }
    return payload;
  }

  static std::uint32_t
  payload_tag(const std::vector<std::uint8_t> &payload) {
    if (payload.empty()) {
      return 0u;
    }

    const std::size_t mid  = payload.size() / 2;
    const std::size_t last = payload.size() - 1;
    return static_cast<std::uint32_t>(payload.size())
           + payload.front() + payload[mid] + payload[last];
  }

  static int patternia_variant_route(const V &v) {
    using namespace ptn;
    using ptn::pat::type::is;

    return match(v)
        .when(is<int>() >> 1)
        .when(is<std::string>() >> 2)
        .when(__ >> 0)
        .end();
  }

  static int patternia_pipe_variant_route(const V &v) {
    using namespace ptn;
    using ptn::pat::type::is;

    return match(v)
           | on{
               is<int>() >> 1,
               is<std::string>() >> 2,
               __ >> 0,
           };
  }

  static int patternia_pipe_variant_alt_route(const V &v) {
    using namespace ptn;

    return match(v)
           | on{
               type::alt<0>() >> 1,
               type::alt<1>() >> 2,
               __ >> 0,
           };
  }

  static int patternia_pipe_variant_alt_32_route(const VAlt32 &v) {
    using namespace ptn;

    return match(v)
           | on{
               type::alt<0>() >> 1,
               type::alt<1>() >> 2,
               type::alt<2>() >> 3,
               type::alt<3>() >> 4,
               type::alt<4>() >> 5,
               type::alt<5>() >> 6,
               type::alt<6>() >> 7,
               type::alt<7>() >> 8,
               type::alt<8>() >> 9,
               type::alt<9>() >> 10,
               type::alt<10>() >> 11,
               type::alt<11>() >> 12,
               type::alt<12>() >> 13,
               type::alt<13>() >> 14,
               type::alt<14>() >> 15,
               type::alt<15>() >> 16,
               type::alt<16>() >> 17,
               type::alt<17>() >> 18,
               type::alt<18>() >> 19,
               type::alt<19>() >> 20,
               type::alt<20>() >> 21,
               type::alt<21>() >> 22,
               type::alt<22>() >> 23,
               type::alt<23>() >> 24,
               type::alt<24>() >> 25,
               type::alt<25>() >> 26,
               type::alt<26>() >> 27,
               type::alt<27>() >> 28,
               type::alt<28>() >> 29,
               type::alt<29>() >> 30,
               type::alt<30>() >> 31,
               type::alt<31>() >> 32,
               __ >> 0,
           };
  }

  static int
  patternia_pipe_variant_alt_32_route_static_cases(const VAlt32 &v) {
    using namespace ptn;

    static auto cases = on{
        type::alt<0>() >> 1,
        type::alt<1>() >> 2,
        type::alt<2>() >> 3,
        type::alt<3>() >> 4,
        type::alt<4>() >> 5,
        type::alt<5>() >> 6,
        type::alt<6>() >> 7,
        type::alt<7>() >> 8,
        type::alt<8>() >> 9,
        type::alt<9>() >> 10,
        type::alt<10>() >> 11,
        type::alt<11>() >> 12,
        type::alt<12>() >> 13,
        type::alt<13>() >> 14,
        type::alt<14>() >> 15,
        type::alt<15>() >> 16,
        type::alt<16>() >> 17,
        type::alt<17>() >> 18,
        type::alt<18>() >> 19,
        type::alt<19>() >> 20,
        type::alt<20>() >> 21,
        type::alt<21>() >> 22,
        type::alt<22>() >> 23,
        type::alt<23>() >> 24,
        type::alt<24>() >> 25,
        type::alt<25>() >> 26,
        type::alt<26>() >> 27,
        type::alt<27>() >> 28,
        type::alt<28>() >> 29,
        type::alt<29>() >> 30,
        type::alt<30>() >> 31,
        type::alt<31>() >> 32,
        __ >> 0,
    };

    return match(v) | cases;
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

  static int sequential_variant_route(const V &v) {
    if (std::holds_alternative<int>(v)) {
      return 1;
    }
    if (std::holds_alternative<std::string>(v)) {
      return 2;
    }
    return 0;
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

  static int switch_index_variant_alt_32_route(const VAlt32 &v) {
    switch (v.index()) {
    case 0:
      return 1;
    case 1:
      return 2;
    case 2:
      return 3;
    case 3:
      return 4;
    case 4:
      return 5;
    case 5:
      return 6;
    case 6:
      return 7;
    case 7:
      return 8;
    case 8:
      return 9;
    case 9:
      return 10;
    case 10:
      return 11;
    case 11:
      return 12;
    case 12:
      return 13;
    case 13:
      return 14;
    case 14:
      return 15;
    case 15:
      return 16;
    case 16:
      return 17;
    case 17:
      return 18;
    case 18:
      return 19;
    case 19:
      return 20;
    case 20:
      return 21;
    case 21:
      return 22;
    case 22:
      return 23;
    case 23:
      return 24;
    case 24:
      return 25;
    case 25:
      return 26;
    case 26:
      return 27;
    case 27:
      return 28;
    case 28:
      return 29;
    case 29:
      return 30;
    case 30:
      return 31;
    case 31:
      return 32;
    default:
      return 0;
    }
  }

  static int patternia_variant_guarded_route(const V &v) {
    using namespace ptn;
    auto long_string = [](const std::string &s) {
      return s.size() > 4;
    };

    return match(v)
        .when(type::as<int>()[_ > 100] >> 10)
        .when(type::is<int>() >> 1)
        .when(type::as<std::string>()[long_string] >> 20)
        .when(type::is<std::string>() >> 2)
        .when(__ >> 0)
        .end();
  }

  static int patternia_pipe_variant_guarded_route(const V &v) {
    using namespace ptn;
    static auto long_string = [](const std::string &s) {
      return s.size() > 4;
    };

    return match(v)
           | on{
               type::as<int>()[_ > 100] >> 10,
               type::is<int>() >> 1,
               type::as<std::string>()[long_string] >> 20,
               type::is<std::string>() >> 2,
               __ >> 0,
           };
  }

  static int std_visit_variant_guarded_route(const V &v) {
    return std::visit(
        [](const auto &x) -> int {
          using T = std::decay_t<decltype(x)>;
          if constexpr (std::is_same_v<T, int>) {
            if (x > 100) {
              return 10;
            }
            return 1;
          }
          else if constexpr (std::is_same_v<T, std::string>) {
            if (x.size() > 4) {
              return 20;
            }
            return 2;
          }
          else {
            return 0;
          }
        },
        v);
  }

  static int sequential_variant_guarded_route(const V &v) {
    if (std::holds_alternative<int>(v)) {
      const int x = std::get<int>(v);
      if (x > 100) {
        return 10;
      }
      return 1;
    }
    if (std::holds_alternative<std::string>(v)) {
      const auto &s = std::get<std::string>(v);
      if (s.size() > 4) {
        return 20;
      }
      return 2;
    }
    return 0;
  }

  static int switch_index_variant_guarded_route(const V &v) {
    switch (v.index()) {
    case 0: {
      const int x = std::get<0>(v);
      if (x > 100) {
        return 10;
      }
      return 1;
    }
    case 1: {
      const auto &s = std::get<1>(v);
      if (s.size() > 4) {
        return 20;
      }
      return 2;
    }
    default:
      return 0;
    }
  }

  static int patternia_protocol_router(const ProtocolMsg &msg) {
    using namespace ptn;
    auto urgent_ping = [](const ProtoPing &p) { return p.urgent; };
    auto heavy_data  = [](const ProtoData &d) {
      return d.reliable && d.length >= 256;
    };
    auto fatal_error = [](const ProtoError &e) { return e.fatal; };
    auto control_ack = [](const ProtoControl &c) {
      return c.op == 1 && c.ack_required;
    };

    return match(msg)
        .when(type::as<ProtoPing>()[urgent_ping] >> 11)
        .when(type::is<ProtoPing>() >> 1)
        .when(type::as<ProtoData>()[heavy_data] >> 22)
        .when(type::is<ProtoData>() >> 2)
        .when(type::as<ProtoError>()[fatal_error] >> 33)
        .when(type::is<ProtoError>() >> 3)
        .when(type::as<ProtoControl>()[control_ack] >> 44)
        .when(type::is<ProtoControl>() >> 4)
        .when(__ >> 0)
        .end();
  }

  static int patternia_pipe_protocol_router(const ProtocolMsg &msg) {
    using namespace ptn;
    auto urgent_ping = [](const ProtoPing &p) { return p.urgent; };
    auto heavy_data  = [](const ProtoData &d) {
      return d.reliable && d.length >= 256;
    };
    auto fatal_error = [](const ProtoError &e) { return e.fatal; };
    auto control_ack = [](const ProtoControl &c) {
      return c.op == 1 && c.ack_required;
    };

    return match(msg)
           | on{
               type::as<ProtoPing>()[urgent_ping] >> 11,
               type::is<ProtoPing>() >> 1,
               type::as<ProtoData>()[heavy_data] >> 22,
               type::is<ProtoData>() >> 2,
               type::as<ProtoError>()[fatal_error] >> 33,
               type::is<ProtoError>() >> 3,
               type::as<ProtoControl>()[control_ack] >> 44,
               type::is<ProtoControl>() >> 4,
               __ >> 0,
           };
  }

  static int if_else_protocol_router(const ProtocolMsg &msg) {
    if (std::holds_alternative<ProtoPing>(msg)) {
      const auto &p = std::get<ProtoPing>(msg);
      if (p.urgent) {
        return 11;
      }
      return 1;
    }
    if (std::holds_alternative<ProtoData>(msg)) {
      const auto &d = std::get<ProtoData>(msg);
      if (d.reliable && d.length >= 256) {
        return 22;
      }
      return 2;
    }
    if (std::holds_alternative<ProtoError>(msg)) {
      const auto &e = std::get<ProtoError>(msg);
      if (e.fatal) {
        return 33;
      }
      return 3;
    }
    if (std::holds_alternative<ProtoControl>(msg)) {
      const auto &c = std::get<ProtoControl>(msg);
      if (c.op == 1 && c.ack_required) {
        return 44;
      }
      return 4;
    }
    return 0;
  }

  static int switch_protocol_router(const ProtocolMsg &msg) {
    switch (msg.index()) {
    case 0: {
      const auto &p = std::get<0>(msg);
      return p.urgent ? 11 : 1;
    }
    case 1: {
      const auto &d = std::get<1>(msg);
      return (d.reliable && d.length >= 256) ? 22 : 2;
    }
    case 2: {
      const auto &e = std::get<2>(msg);
      return e.fatal ? 33 : 3;
    }
    case 3: {
      const auto &c = std::get<3>(msg);
      return (c.op == 1 && c.ack_required) ? 44 : 4;
    }
    default:
      return 0;
    }
  }

  static int std_visit_protocol_router(const ProtocolMsg &msg) {
    return std::visit(
        [](const auto &x) -> int {
          using T = std::decay_t<decltype(x)>;
          if constexpr (std::is_same_v<T, ProtoPing>) {
            return x.urgent ? 11 : 1;
          }
          else if constexpr (std::is_same_v<T, ProtoData>) {
            return (x.reliable && x.length >= 256) ? 22 : 2;
          }
          else if constexpr (std::is_same_v<T, ProtoError>) {
            return x.fatal ? 33 : 3;
          }
          else if constexpr (std::is_same_v<T, ProtoControl>) {
            return (x.op == 1 && x.ack_required) ? 44 : 4;
          }
          else {
            return 0;
          }
        },
        msg);
  }

  static int patternia_command_parser(const CommandMsg &msg) {
    using namespace ptn;
    auto persistent_set = [](const CmdSet &c) {
      return c.persist && c.value >= 0;
    };
    auto hot_get = [](const CmdGet &c) {
      return c.allow_stale && c.key < 256;
    };
    auto deep_del  = [](const CmdDel &c) { return c.recursive; };
    auto wide_scan = [](const CmdScan &c) { return c.limit >= 128; };

    return match(msg)
        .when(type::as<CmdSet>()[persistent_set] >> 101)
        .when(type::is<CmdSet>() >> 100)
        .when(type::as<CmdGet>()[hot_get] >> 201)
        .when(type::is<CmdGet>() >> 200)
        .when(type::as<CmdDel>()[deep_del] >> 301)
        .when(type::is<CmdDel>() >> 300)
        .when(type::as<CmdScan>()[wide_scan] >> 401)
        .when(type::is<CmdScan>() >> 400)
        .when(__ >> 0)
        .end();
  }

  static int patternia_pipe_command_parser(const CommandMsg &msg) {
    using namespace ptn;
    auto persistent_set = [](const CmdSet &c) {
      return c.persist && c.value >= 0;
    };
    auto hot_get = [](const CmdGet &c) {
      return c.allow_stale && c.key < 256;
    };
    auto deep_del  = [](const CmdDel &c) { return c.recursive; };
    auto wide_scan = [](const CmdScan &c) { return c.limit >= 128; };

    return match(msg)
           | on{
               type::as<CmdSet>()[persistent_set] >> 101,
               type::is<CmdSet>() >> 100,
               type::as<CmdGet>()[hot_get] >> 201,
               type::is<CmdGet>() >> 200,
               type::as<CmdDel>()[deep_del] >> 301,
               type::is<CmdDel>() >> 300,
               type::as<CmdScan>()[wide_scan] >> 401,
               type::is<CmdScan>() >> 400,
               __ >> 0,
           };
  }

  static int if_else_command_parser(const CommandMsg &msg) {
    if (std::holds_alternative<CmdSet>(msg)) {
      const auto &c = std::get<CmdSet>(msg);
      if (c.persist && c.value >= 0) {
        return 101;
      }
      return 100;
    }
    if (std::holds_alternative<CmdGet>(msg)) {
      const auto &c = std::get<CmdGet>(msg);
      if (c.allow_stale && c.key < 256) {
        return 201;
      }
      return 200;
    }
    if (std::holds_alternative<CmdDel>(msg)) {
      const auto &c = std::get<CmdDel>(msg);
      if (c.recursive) {
        return 301;
      }
      return 300;
    }
    if (std::holds_alternative<CmdScan>(msg)) {
      const auto &c = std::get<CmdScan>(msg);
      if (c.limit >= 128) {
        return 401;
      }
      return 400;
    }
    return 0;
  }

  static int switch_command_parser(const CommandMsg &msg) {
    switch (msg.index()) {
    case 0: {
      const auto &c = std::get<0>(msg);
      return (c.persist && c.value >= 0) ? 101 : 100;
    }
    case 1: {
      const auto &c = std::get<1>(msg);
      return (c.allow_stale && c.key < 256) ? 201 : 200;
    }
    case 2: {
      const auto &c = std::get<2>(msg);
      return c.recursive ? 301 : 300;
    }
    case 3: {
      const auto &c = std::get<3>(msg);
      return c.limit >= 128 ? 401 : 400;
    }
    default:
      return 0;
    }
  }

  static int std_visit_command_parser(const CommandMsg &msg) {
    return std::visit(
        [](const auto &x) -> int {
          using T = std::decay_t<decltype(x)>;
          if constexpr (std::is_same_v<T, CmdSet>) {
            return (x.persist && x.value >= 0) ? 101 : 100;
          }
          else if constexpr (std::is_same_v<T, CmdGet>) {
            return (x.allow_stale && x.key < 256) ? 201 : 200;
          }
          else if constexpr (std::is_same_v<T, CmdDel>) {
            return x.recursive ? 301 : 300;
          }
          else if constexpr (std::is_same_v<T, CmdScan>) {
            return x.limit >= 128 ? 401 : 400;
          }
          else {
            return 0;
          }
        },
        msg);
  }

  static int patternia_pipe_literal_match_route(int x) {
    using namespace ptn;

    return match(x)
           | on{
               lit(1) >> 1,
               lit(2) >> 2,
               lit(3) >> 3,
               lit(4) >> 4,
               lit(5) >> 5,
               lit(6) >> 6,
               lit(7) >> 7,
               lit(8) >> 8,
               __ >> 0,
           };
  }

  static int patternia_literal_match_route(int x) {
    using namespace ptn;

    return match(x)
        .when(lit(1) >> 1)
        .when(lit(2) >> 2)
        .when(lit(3) >> 3)
        .when(lit(4) >> 4)
        .when(lit(5) >> 5)
        .when(lit(6) >> 6)
        .when(lit(7) >> 7)
        .when(lit(8) >> 8)
        .when(__ >> 0)
        .end();
  }

  static int if_else_literal_match_route(int x) {
    if (x == 1) {
      return 1;
    }
    if (x == 2) {
      return 2;
    }
    if (x == 3) {
      return 3;
    }
    if (x == 4) {
      return 4;
    }
    if (x == 5) {
      return 5;
    }
    if (x == 6) {
      return 6;
    }
    if (x == 7) {
      return 7;
    }
    if (x == 8) {
      return 8;
    }
    return 0;
  }

  static int switch_literal_match_route(int x) {
    switch (x) {
    case 1:
      return 1;
    case 2:
      return 2;
    case 3:
      return 3;
    case 4:
      return 4;
    case 5:
      return 5;
    case 6:
      return 6;
    case 7:
      return 7;
    case 8:
      return 8;
    default:
      return 0;
    }
  }

#define PTN_LIT_CASE_128(n) ptn::lit(n) >> (n)
#define PTN_SWITCH_CASE_128(n)                                      \
  case (n):                                                         \
    return (n)

#define PTN_LIT_BLOCK_16(base)                                      \
  PTN_LIT_CASE_128((base) + 0), PTN_LIT_CASE_128((base) + 1),       \
      PTN_LIT_CASE_128((base) + 2), PTN_LIT_CASE_128((base) + 3),   \
      PTN_LIT_CASE_128((base) + 4), PTN_LIT_CASE_128((base) + 5),   \
      PTN_LIT_CASE_128((base) + 6), PTN_LIT_CASE_128((base) + 7),   \
      PTN_LIT_CASE_128((base) + 8), PTN_LIT_CASE_128((base) + 9),   \
      PTN_LIT_CASE_128((base) + 10), PTN_LIT_CASE_128((base) + 11), \
      PTN_LIT_CASE_128((base) + 12), PTN_LIT_CASE_128((base) + 13), \
      PTN_LIT_CASE_128((base) + 14), PTN_LIT_CASE_128((base) + 15)

#define PTN_SWITCH_BLOCK_16(base)                                   \
  PTN_SWITCH_CASE_128((base) + 0);                                  \
  PTN_SWITCH_CASE_128((base) + 1);                                  \
  PTN_SWITCH_CASE_128((base) + 2);                                  \
  PTN_SWITCH_CASE_128((base) + 3);                                  \
  PTN_SWITCH_CASE_128((base) + 4);                                  \
  PTN_SWITCH_CASE_128((base) + 5);                                  \
  PTN_SWITCH_CASE_128((base) + 6);                                  \
  PTN_SWITCH_CASE_128((base) + 7);                                  \
  PTN_SWITCH_CASE_128((base) + 8);                                  \
  PTN_SWITCH_CASE_128((base) + 9);                                  \
  PTN_SWITCH_CASE_128((base) + 10);                                 \
  PTN_SWITCH_CASE_128((base) + 11);                                 \
  PTN_SWITCH_CASE_128((base) + 12);                                 \
  PTN_SWITCH_CASE_128((base) + 13);                                 \
  PTN_SWITCH_CASE_128((base) + 14);                                 \
  PTN_SWITCH_CASE_128((base) + 15)

  static int patternia_pipe_literal_match_128_route(int x) {
    using namespace ptn;

    return match(x)
           | on{
               PTN_LIT_BLOCK_16(1),
               PTN_LIT_BLOCK_16(17),
               PTN_LIT_BLOCK_16(33),
               PTN_LIT_BLOCK_16(49),
               PTN_LIT_BLOCK_16(65),
               PTN_LIT_BLOCK_16(81),
               PTN_LIT_BLOCK_16(97),
               PTN_LIT_BLOCK_16(113),
               __ >> 0,
           };
  }

  static int switch_literal_match_128_route(int x) {
    switch (x) {
      PTN_SWITCH_BLOCK_16(1);
      PTN_SWITCH_BLOCK_16(17);
      PTN_SWITCH_BLOCK_16(33);
      PTN_SWITCH_BLOCK_16(49);
      PTN_SWITCH_BLOCK_16(65);
      PTN_SWITCH_BLOCK_16(81);
      PTN_SWITCH_BLOCK_16(97);
      PTN_SWITCH_BLOCK_16(113);
    default:
      return 0;
    }
  }

  static const std::vector<int> &literal_128_workload() {
    static const std::vector<int> data = [] {
      std::vector<int> v;
      v.reserve(260);

      for (int i = 1; i <= 128; ++i) {
        v.push_back(i);
      }

      v.push_back(0);
      v.push_back(129);
      v.push_back(999);

      for (int i = 128; i >= 1; --i) {
        v.push_back(i);
      }

      return v;
    }();

    return data;
  }

  template <typename F>
  static void run_variant_alternating_hot(benchmark::State &state,
                                          F                 fn) {
    // Microbench: isolate dispatch overhead with two prebuilt
    // alternatives.
    static V int_alt = 7;
    static V str_alt = std::string("patternia");

    bool pick_str = false;
    int  acc      = 0;

    for (auto _ : state) {
      V &v     = pick_str ? str_alt : int_alt;
      pick_str = !pick_str;

      benchmark::DoNotOptimize(v);
      acc += fn(v);
      benchmark::DoNotOptimize(acc);
      benchmark::ClobberMemory();
    }

    benchmark::DoNotOptimize(acc);
    state.SetItemsProcessed(state.iterations());
  }

  template <typename F>
  static void
  run_variant_alt_32_alternating_hot(benchmark::State &state, F fn) {
    // Microbench: isolate dispatch overhead across far-apart
    // alternatives.
    static VAlt32 low_alt  = VAlt32{std::in_place_index<0>,
                                   VariantAltToken<0>{0}};
    static VAlt32 high_alt = VAlt32{std::in_place_index<31>,
                                    VariantAltToken<31>{31}};

    bool pick_high = false;
    int  acc       = 0;

    for (auto _ : state) {
      VAlt32 &v = pick_high ? high_alt : low_alt;
      pick_high = !pick_high;

      benchmark::DoNotOptimize(v);
      acc += fn(v);
      benchmark::DoNotOptimize(acc);
      benchmark::ClobberMemory();
    }

    benchmark::DoNotOptimize(acc);
    state.SetItemsProcessed(state.iterations());
  }

  static int patternia_packet_route(const Packet &pkt) {
    using namespace ptn;

    auto is_ping_packet = [](std::uint8_t  type,
                             std::uint16_t length) {
      return type == 0x01 && length == 0;
    };

    auto is_valid_data_packet = [&pkt](std::uint8_t  type,
                                       std::uint16_t length,
                                       std::uint8_t  flags) {
      return type == 0x02 && length == pkt.payload.size()
             && (flags & FLAG_VALID);
    };

    auto is_error_packet = [&pkt](std::uint8_t type) {
      return type == 0xFF && !pkt.payload.empty();
    };

    return match(pkt)
        .when(bind(has<&Packet::type,
                       &Packet::length>())[is_ping_packet]
              >> 1)
        .when(bind(has<&Packet::type,
                       &Packet::length,
                       &Packet::flags>())[is_valid_data_packet]
              >> 2)
        .when(bind(has<&Packet::type>())[is_error_packet] >> 3)
        .otherwise(0);
  }

  static int patternia_pipe_packet_route(const Packet &pkt) {
    using namespace ptn;

    auto is_ping_packet = [](std::uint8_t  type,
                             std::uint16_t length) {
      return type == 0x01 && length == 0;
    };

    auto is_valid_data_packet = [&pkt](std::uint8_t  type,
                                       std::uint16_t length,
                                       std::uint8_t  flags) {
      return type == 0x02 && length == pkt.payload.size()
             && (flags & FLAG_VALID);
    };

    auto is_error_packet = [&pkt](std::uint8_t type) {
      return type == 0xFF && !pkt.payload.empty();
    };

    return match(pkt)
           | on{
               bind(has<&Packet::type,
                        &Packet::length>())[is_ping_packet]
                   >> 1,
               bind(has<&Packet::type,
                        &Packet::length,
                        &Packet::flags>())[is_valid_data_packet]
                   >> 2,
               bind(has<&Packet::type>())[is_error_packet] >> 3,
               __ >> 0,
           };
  }

  static int switch_packet_route(const Packet &pkt) {
    switch (pkt.type) {
    case 0x01:
      if (pkt.length == 0) {
        return 1;
      }
      break;
    case 0x02:
      if (pkt.length == pkt.payload.size()
          && (pkt.flags & FLAG_VALID)) {
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

  static int patternia_packet_heavy_bind_route(const Packet &pkt) {
    using namespace ptn;

    auto is_ping_packet = [](std::uint8_t  type,
                             std::uint16_t length) {
      return type == 0x01 && length == 0;
    };

    auto is_valid_data_packet =
        [](std::uint8_t                     type,
           std::uint16_t                    length,
           std::uint8_t                     flags,
           const std::vector<std::uint8_t> &payload) {
          return type == 0x02 && length == payload.size()
                 && (flags & FLAG_VALID) != 0u
                 && payload_tag(payload) != 0u;
        };

    auto is_error_packet =
        [](std::uint8_t                     type,
           const std::vector<std::uint8_t> &payload) {
          return type == 0xFF && payload_tag(payload) > 0u;
        };

    return match(pkt)
        .when(bind(has<&Packet::type,
                       &Packet::length>())[is_ping_packet]
              >> 1)
        .when(bind(has<&Packet::type,
                       &Packet::length,
                       &Packet::flags,
                       &Packet::payload>())[is_valid_data_packet]
              >> 2)
        .when(bind(has<&Packet::type,
                       &Packet::payload>())[is_error_packet]
              >> 3)
        .otherwise(0);
  }

  static int
  patternia_pipe_packet_heavy_bind_route(const Packet &pkt) {
    using namespace ptn;

    auto is_ping_packet = [](std::uint8_t  type,
                             std::uint16_t length) {
      return type == 0x01 && length == 0;
    };

    auto is_valid_data_packet =
        [](std::uint8_t                     type,
           std::uint16_t                    length,
           std::uint8_t                     flags,
           const std::vector<std::uint8_t> &payload) {
          return type == 0x02 && length == payload.size()
                 && (flags & FLAG_VALID) != 0u
                 && payload_tag(payload) != 0u;
        };

    auto is_error_packet =
        [](std::uint8_t                     type,
           const std::vector<std::uint8_t> &payload) {
          return type == 0xFF && payload_tag(payload) > 0u;
        };

    return match(pkt)
           | on{
               bind(has<&Packet::type,
                        &Packet::length>())[is_ping_packet]
                   >> 1,
               bind(has<&Packet::type,
                        &Packet::length,
                        &Packet::flags,
                        &Packet::payload>())[is_valid_data_packet]
                   >> 2,
               bind(has<&Packet::type,
                        &Packet::payload>())[is_error_packet]
                   >> 3,
               __ >> 0,
           };
  }

  static int switch_packet_heavy_bind_route(const Packet &pkt) {
    switch (pkt.type) {
    case 0x01:
      if (pkt.length == 0) {
        return 1;
      }
      break;
    case 0x02:
      if (pkt.length == pkt.payload.size()
          && (pkt.flags & FLAG_VALID) != 0u
          && payload_tag(pkt.payload) != 0u) {
        return 2;
      }
      break;
    case 0xFF:
      if (payload_tag(pkt.payload) > 0u) {
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

  static const std::vector<VAlt32> &variant_alt_32_workload() {
    static const std::vector<VAlt32> data = {
        VAlt32{std::in_place_index<0>, VariantAltToken<0>{0}},
        VAlt32{std::in_place_index<1>, VariantAltToken<1>{1}},
        VAlt32{std::in_place_index<2>, VariantAltToken<2>{2}},
        VAlt32{std::in_place_index<3>, VariantAltToken<3>{3}},
        VAlt32{std::in_place_index<4>, VariantAltToken<4>{4}},
        VAlt32{std::in_place_index<5>, VariantAltToken<5>{5}},
        VAlt32{std::in_place_index<6>, VariantAltToken<6>{6}},
        VAlt32{std::in_place_index<7>, VariantAltToken<7>{7}},
        VAlt32{std::in_place_index<8>, VariantAltToken<8>{8}},
        VAlt32{std::in_place_index<9>, VariantAltToken<9>{9}},
        VAlt32{std::in_place_index<10>, VariantAltToken<10>{10}},
        VAlt32{std::in_place_index<11>, VariantAltToken<11>{11}},
        VAlt32{std::in_place_index<12>, VariantAltToken<12>{12}},
        VAlt32{std::in_place_index<13>, VariantAltToken<13>{13}},
        VAlt32{std::in_place_index<14>, VariantAltToken<14>{14}},
        VAlt32{std::in_place_index<15>, VariantAltToken<15>{15}},
        VAlt32{std::in_place_index<16>, VariantAltToken<16>{16}},
        VAlt32{std::in_place_index<17>, VariantAltToken<17>{17}},
        VAlt32{std::in_place_index<18>, VariantAltToken<18>{18}},
        VAlt32{std::in_place_index<19>, VariantAltToken<19>{19}},
        VAlt32{std::in_place_index<20>, VariantAltToken<20>{20}},
        VAlt32{std::in_place_index<21>, VariantAltToken<21>{21}},
        VAlt32{std::in_place_index<22>, VariantAltToken<22>{22}},
        VAlt32{std::in_place_index<23>, VariantAltToken<23>{23}},
        VAlt32{std::in_place_index<24>, VariantAltToken<24>{24}},
        VAlt32{std::in_place_index<25>, VariantAltToken<25>{25}},
        VAlt32{std::in_place_index<26>, VariantAltToken<26>{26}},
        VAlt32{std::in_place_index<27>, VariantAltToken<27>{27}},
        VAlt32{std::in_place_index<28>, VariantAltToken<28>{28}},
        VAlt32{std::in_place_index<29>, VariantAltToken<29>{29}},
        VAlt32{std::in_place_index<30>, VariantAltToken<30>{30}},
        VAlt32{std::in_place_index<31>, VariantAltToken<31>{31}},
        VAlt32{std::in_place_index<31>, VariantAltToken<31>{31}},
        VAlt32{std::in_place_index<30>, VariantAltToken<30>{30}},
        VAlt32{std::in_place_index<29>, VariantAltToken<29>{29}},
        VAlt32{std::in_place_index<28>, VariantAltToken<28>{28}},
        VAlt32{std::in_place_index<27>, VariantAltToken<27>{27}},
        VAlt32{std::in_place_index<26>, VariantAltToken<26>{26}},
        VAlt32{std::in_place_index<25>, VariantAltToken<25>{25}},
        VAlt32{std::in_place_index<24>, VariantAltToken<24>{24}},
        VAlt32{std::in_place_index<23>, VariantAltToken<23>{23}},
        VAlt32{std::in_place_index<22>, VariantAltToken<22>{22}},
        VAlt32{std::in_place_index<21>, VariantAltToken<21>{21}},
        VAlt32{std::in_place_index<20>, VariantAltToken<20>{20}},
        VAlt32{std::in_place_index<19>, VariantAltToken<19>{19}},
        VAlt32{std::in_place_index<18>, VariantAltToken<18>{18}},
        VAlt32{std::in_place_index<17>, VariantAltToken<17>{17}},
        VAlt32{std::in_place_index<16>, VariantAltToken<16>{16}},
        VAlt32{std::in_place_index<15>, VariantAltToken<15>{15}},
        VAlt32{std::in_place_index<14>, VariantAltToken<14>{14}},
        VAlt32{std::in_place_index<13>, VariantAltToken<13>{13}},
        VAlt32{std::in_place_index<12>, VariantAltToken<12>{12}},
        VAlt32{std::in_place_index<11>, VariantAltToken<11>{11}},
        VAlt32{std::in_place_index<10>, VariantAltToken<10>{10}},
        VAlt32{std::in_place_index<9>, VariantAltToken<9>{9}},
        VAlt32{std::in_place_index<8>, VariantAltToken<8>{8}},
        VAlt32{std::in_place_index<7>, VariantAltToken<7>{7}},
        VAlt32{std::in_place_index<6>, VariantAltToken<6>{6}},
        VAlt32{std::in_place_index<5>, VariantAltToken<5>{5}},
        VAlt32{std::in_place_index<4>, VariantAltToken<4>{4}},
        VAlt32{std::in_place_index<3>, VariantAltToken<3>{3}},
        VAlt32{std::in_place_index<2>, VariantAltToken<2>{2}},
        VAlt32{std::in_place_index<1>, VariantAltToken<1>{1}},
        VAlt32{std::in_place_index<0>, VariantAltToken<0>{0}},
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

  static const std::vector<Packet> &packet_heavy_workload() {
    static const std::vector<Packet> data = {
        {0x01, 0, 0, {}},
        {0x02, 256, FLAG_VALID, make_payload(256, 1)},
        {0xFF, 1024, 0, make_payload(1024, 3)},
        {0x02, 1024, FLAG_VALID, make_payload(1024, 7)},
        {0x02, 4096, FLAG_VALID, make_payload(4096, 11)},
        {0x02, 4096, 0, make_payload(4096, 13)},
        {0xFF, 0, 0, {}},
        {0x02, 512, FLAG_VALID, make_payload(511, 5)},
        {0xFF, 2048, 0, make_payload(2048, 9)},
        {0x01, 1, 0, make_payload(1, 2)},
    };
    return data;
  }

  static const std::vector<ProtocolMsg> &protocol_workload() {
    static const std::vector<ProtocolMsg> data = {
        ProtoPing{1, false},
        ProtoPing{2, true},
        ProtoData{3, 64, false},
        ProtoData{7, 512, true},
        ProtoError{101, false},
        ProtoControl{1, 2, true},
        ProtoControl{2, 1, false},
        ProtoError{999, true},
        ProtoData{4, 300, true},
        ProtoPing{3, false},
        ProtoControl{1, 9, false},
        ProtoData{6, 128, true},
        ProtoError{500, false},
        ProtoPing{4, true},
        ProtoControl{1, 3, true},
        ProtoData{9, 1024, true},
    };
    return data;
  }

  static const std::vector<CommandMsg> &command_workload() {
    static const std::vector<CommandMsg> data = {
        CmdSet{10, 1, true},
        CmdSet{11, -3, true},
        CmdGet{5, true},
        CmdGet{1024, false},
        CmdDel{8, false},
        CmdDel{9, true},
        CmdScan{1, 32},
        CmdScan{2, 256},
        CmdSet{20, 0, false},
        CmdGet{42, true},
        CmdDel{12, false},
        CmdScan{7, 512},
        CmdSet{99, 100, true},
        CmdGet{900, true},
        CmdDel{77, true},
        CmdScan{3, 64},
    };
    return data;
  }

  static const std::vector<int> &literal_workload() {
    static const std::vector<int> data = {
        1,
        2,
        3,
        4,
        5,
        6,
        7,
        8,
        0,
        9,
        2,
        4,
        6,
        8,
        10,
        3,
    };
    return data;
  }

  template <typename T, typename F>
  static void run_workload(benchmark::State     &state,
                           const std::vector<T> &workload,
                           F                     fn) {
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

  static void
  BM_PatterniaPipe_VariantMixed(benchmark::State &state) {
    run_workload(
        state, variant_workload(), patternia_pipe_variant_route);
  }

  static void BM_StdVisit_VariantMixed(benchmark::State &state) {
    run_workload(state, variant_workload(), std_visit_variant_route);
  }

  static void BM_Sequential_VariantMixed(benchmark::State &state) {
    run_workload(
        state, variant_workload(), sequential_variant_route);
  }

  static void BM_SwitchIndex_VariantMixed(benchmark::State &state) {
    run_workload(
        state, variant_workload(), switch_index_variant_route);
  }

  static void BM_Patternia_VariantAltHot(benchmark::State &state) {
    run_variant_alternating_hot(state, patternia_variant_route);
  }

  static void
  BM_PatterniaPipe_VariantAltHot(benchmark::State &state) {
    run_variant_alternating_hot(state, patternia_pipe_variant_route);
  }

  static void BM_StdVisit_VariantAltHot(benchmark::State &state) {
    run_variant_alternating_hot(state, std_visit_variant_route);
  }

  static void BM_Sequential_VariantAltHot(benchmark::State &state) {
    run_variant_alternating_hot(state, sequential_variant_route);
  }

  static void BM_SwitchIndex_VariantAltHot(benchmark::State &state) {
    run_variant_alternating_hot(state, switch_index_variant_route);
  }

  static void
  BM_PatterniaPipe_VariantFastPathMixed(benchmark::State &state) {
    run_workload(
        state, variant_workload(), patternia_pipe_variant_route);
  }

  static void
  BM_PatterniaPipe_VariantFastPathAltHot(benchmark::State &state) {
    run_variant_alternating_hot(state, patternia_pipe_variant_route);
  }

  static void
  BM_PatterniaPipe_VariantAltIndexMixed(benchmark::State &state) {
    run_workload(
        state, variant_workload(), patternia_pipe_variant_alt_route);
  }

  static void
  BM_PatterniaPipe_VariantAltIndexAltHot(benchmark::State &state) {
    run_variant_alternating_hot(state,
                                patternia_pipe_variant_alt_route);
  }

  static void
  BM_PatterniaPipe_VariantAltIndex32Mixed(benchmark::State &state) {
    run_workload(state,
                 variant_alt_32_workload(),
                 patternia_pipe_variant_alt_32_route);
  }

  static void BM_PatterniaPipe_VariantAltIndex32MixedStatic(
      benchmark::State &state) {
    run_workload(state,
                 variant_alt_32_workload(),
                 patternia_pipe_variant_alt_32_route_static_cases);
  }

  static void
  BM_SwitchIndex_VariantAltIndex32Mixed(benchmark::State &state) {
    run_workload(state,
                 variant_alt_32_workload(),
                 switch_index_variant_alt_32_route);
  }

  static void
  BM_PatterniaPipe_VariantAltIndex32AltHot(benchmark::State &state) {
    run_variant_alt_32_alternating_hot(
        state, patternia_pipe_variant_alt_32_route);
  }

  static void BM_PatterniaPipe_VariantAltIndex32AltHotStatic(
      benchmark::State &state) {
    run_variant_alt_32_alternating_hot(
        state, patternia_pipe_variant_alt_32_route_static_cases);
  }

  static void
  BM_SwitchIndex_VariantAltIndex32AltHot(benchmark::State &state) {
    run_variant_alt_32_alternating_hot(
        state, switch_index_variant_alt_32_route);
  }

  static void
  BM_Patternia_VariantMixedGuarded(benchmark::State &state) {
    run_workload(
        state, variant_workload(), patternia_variant_guarded_route);
  }

  static void
  BM_PatterniaPipe_VariantMixedGuarded(benchmark::State &state) {
    run_workload(state,
                 variant_workload(),
                 patternia_pipe_variant_guarded_route);
  }

  static void
  BM_StdVisit_VariantMixedGuarded(benchmark::State &state) {
    run_workload(
        state, variant_workload(), std_visit_variant_guarded_route);
  }

  static void
  BM_Sequential_VariantMixedGuarded(benchmark::State &state) {
    run_workload(
        state, variant_workload(), sequential_variant_guarded_route);
  }

  static void
  BM_SwitchIndex_VariantMixedGuarded(benchmark::State &state) {
    run_workload(state,
                 variant_workload(),
                 switch_index_variant_guarded_route);
  }

  static void
  BM_Patternia_VariantAltHotGuarded(benchmark::State &state) {
    run_variant_alternating_hot(state,
                                patternia_variant_guarded_route);
  }

  static void
  BM_PatterniaPipe_VariantAltHotGuarded(benchmark::State &state) {
    run_variant_alternating_hot(
        state, patternia_pipe_variant_guarded_route);
  }

  static void
  BM_StdVisit_VariantAltHotGuarded(benchmark::State &state) {
    run_variant_alternating_hot(state,
                                std_visit_variant_guarded_route);
  }

  static void
  BM_Sequential_VariantAltHotGuarded(benchmark::State &state) {
    run_variant_alternating_hot(state,
                                sequential_variant_guarded_route);
  }

  static void
  BM_SwitchIndex_VariantAltHotGuarded(benchmark::State &state) {
    run_variant_alternating_hot(state,
                                switch_index_variant_guarded_route);
  }

  static void BM_Patternia_PacketMixed(benchmark::State &state) {
    run_workload(state, packet_workload(), patternia_packet_route);
  }

  static void BM_PatterniaPipe_PacketMixed(benchmark::State &state) {
    run_workload(
        state, packet_workload(), patternia_pipe_packet_route);
  }

  static void BM_Switch_PacketMixed(benchmark::State &state) {
    run_workload(state, packet_workload(), switch_packet_route);
  }

  static void
  BM_Patternia_PacketMixedHeavyBind(benchmark::State &state) {
    run_workload(state,
                 packet_heavy_workload(),
                 patternia_packet_heavy_bind_route);
  }

  static void
  BM_PatterniaPipe_PacketMixedHeavyBind(benchmark::State &state) {
    run_workload(state,
                 packet_heavy_workload(),
                 patternia_pipe_packet_heavy_bind_route);
  }

  static void
  BM_Switch_PacketMixedHeavyBind(benchmark::State &state) {
    run_workload(state,
                 packet_heavy_workload(),
                 switch_packet_heavy_bind_route);
  }

  static void BM_Patternia_ProtocolRouter(benchmark::State &state) {
    run_workload(
        state, protocol_workload(), patternia_protocol_router);
  }

  static void
  BM_PatterniaPipe_ProtocolRouter(benchmark::State &state) {
    run_workload(
        state, protocol_workload(), patternia_pipe_protocol_router);
  }

  static void BM_IfElse_ProtocolRouter(benchmark::State &state) {
    run_workload(
        state, protocol_workload(), if_else_protocol_router);
  }

  static void BM_Switch_ProtocolRouter(benchmark::State &state) {
    run_workload(state, protocol_workload(), switch_protocol_router);
  }

  static void BM_StdVisit_ProtocolRouter(benchmark::State &state) {
    run_workload(
        state, protocol_workload(), std_visit_protocol_router);
  }

  static void BM_Patternia_CommandParser(benchmark::State &state) {
    run_workload(
        state, command_workload(), patternia_command_parser);
  }

  static void
  BM_PatterniaPipe_CommandParser(benchmark::State &state) {
    run_workload(
        state, command_workload(), patternia_pipe_command_parser);
  }

  static void BM_IfElse_CommandParser(benchmark::State &state) {
    run_workload(state, command_workload(), if_else_command_parser);
  }

  static void BM_Switch_CommandParser(benchmark::State &state) {
    run_workload(state, command_workload(), switch_command_parser);
  }

  static void BM_StdVisit_CommandParser(benchmark::State &state) {
    run_workload(
        state, command_workload(), std_visit_command_parser);
  }

  static void
  BM_PatterniaPipe_LiteralMatch(benchmark::State &state) {
    run_workload(state,
                 literal_workload(),
                 patternia_pipe_literal_match_route);
  }

  static void BM_Patternia_LiteralMatch(benchmark::State &state) {
    run_workload(
        state, literal_workload(), patternia_literal_match_route);
  }

  static void BM_IfElse_LiteralMatch(benchmark::State &state) {
    run_workload(
        state, literal_workload(), if_else_literal_match_route);
  }

  static void BM_Switch_LiteralMatch(benchmark::State &state) {
    run_workload(
        state, literal_workload(), switch_literal_match_route);
  }

  static void
  BM_PatterniaPipe_LiteralMatch128(benchmark::State &state) {
    run_workload(state,
                 literal_128_workload(),
                 patternia_pipe_literal_match_128_route);
  }

  static void BM_Switch_LiteralMatch128(benchmark::State &state) {
    run_workload(state,
                 literal_128_workload(),
                 switch_literal_match_128_route);
  }

} // namespace

#define PTN_REGISTER_STABLE_BENCH(name)                             \
  BENCHMARK(name)                                                   \
      ->Unit(benchmark::kNanosecond)                                \
      ->MinTime(0.5)                                                \
      ->Repetitions(20)                                             \
      ->ReportAggregatesOnly(true)

#if PTN_BENCH_MODE == PTN_BENCH_MODE_PIPE_STD
#if PTN_BENCH_ENABLE_SUITE_VARIANT
PTN_REGISTER_STABLE_BENCH(BM_PatterniaPipe_VariantMixed);
PTN_REGISTER_STABLE_BENCH(BM_StdVisit_VariantMixed);
PTN_REGISTER_STABLE_BENCH(BM_Sequential_VariantMixed);
PTN_REGISTER_STABLE_BENCH(BM_SwitchIndex_VariantMixed);
PTN_REGISTER_STABLE_BENCH(BM_PatterniaPipe_VariantAltHot);
PTN_REGISTER_STABLE_BENCH(BM_StdVisit_VariantAltHot);
PTN_REGISTER_STABLE_BENCH(BM_Sequential_VariantAltHot);
PTN_REGISTER_STABLE_BENCH(BM_SwitchIndex_VariantAltHot);
#endif

#if PTN_BENCH_ENABLE_SUITE_VARIANT_FASTPATH
PTN_REGISTER_STABLE_BENCH(BM_PatterniaPipe_VariantFastPathMixed);
PTN_REGISTER_STABLE_BENCH(BM_PatterniaPipe_VariantFastPathAltHot);
PTN_REGISTER_STABLE_BENCH(BM_PatterniaPipe_VariantAltIndexMixed);
PTN_REGISTER_STABLE_BENCH(BM_PatterniaPipe_VariantAltIndexAltHot);
PTN_REGISTER_STABLE_BENCH(BM_PatterniaPipe_VariantAltIndex32Mixed);
PTN_REGISTER_STABLE_BENCH(
    BM_PatterniaPipe_VariantAltIndex32MixedStatic);
PTN_REGISTER_STABLE_BENCH(BM_SwitchIndex_VariantAltIndex32Mixed);
PTN_REGISTER_STABLE_BENCH(BM_PatterniaPipe_VariantAltIndex32AltHot);
PTN_REGISTER_STABLE_BENCH(
    BM_PatterniaPipe_VariantAltIndex32AltHotStatic);
PTN_REGISTER_STABLE_BENCH(BM_SwitchIndex_VariantAltIndex32AltHot);
#endif

#if PTN_BENCH_ENABLE_SUITE_VARIANT_GUARDED
PTN_REGISTER_STABLE_BENCH(BM_PatterniaPipe_VariantMixedGuarded);
PTN_REGISTER_STABLE_BENCH(BM_StdVisit_VariantMixedGuarded);
PTN_REGISTER_STABLE_BENCH(BM_Sequential_VariantMixedGuarded);
PTN_REGISTER_STABLE_BENCH(BM_SwitchIndex_VariantMixedGuarded);
PTN_REGISTER_STABLE_BENCH(BM_PatterniaPipe_VariantAltHotGuarded);
PTN_REGISTER_STABLE_BENCH(BM_StdVisit_VariantAltHotGuarded);
PTN_REGISTER_STABLE_BENCH(BM_Sequential_VariantAltHotGuarded);
PTN_REGISTER_STABLE_BENCH(BM_SwitchIndex_VariantAltHotGuarded);
#endif

#if PTN_BENCH_ENABLE_SUITE_PROTOCOL_ROUTER
PTN_REGISTER_STABLE_BENCH(BM_PatterniaPipe_ProtocolRouter);
PTN_REGISTER_STABLE_BENCH(BM_IfElse_ProtocolRouter);
PTN_REGISTER_STABLE_BENCH(BM_Switch_ProtocolRouter);
PTN_REGISTER_STABLE_BENCH(BM_StdVisit_ProtocolRouter);
#endif

#if PTN_BENCH_ENABLE_SUITE_COMMAND_PARSER
PTN_REGISTER_STABLE_BENCH(BM_PatterniaPipe_CommandParser);
PTN_REGISTER_STABLE_BENCH(BM_IfElse_CommandParser);
PTN_REGISTER_STABLE_BENCH(BM_Switch_CommandParser);
PTN_REGISTER_STABLE_BENCH(BM_StdVisit_CommandParser);
#endif

#if PTN_BENCH_ENABLE_SUITE_LITERAL_MATCH
PTN_REGISTER_STABLE_BENCH(BM_PatterniaPipe_LiteralMatch);
PTN_REGISTER_STABLE_BENCH(BM_IfElse_LiteralMatch);
PTN_REGISTER_STABLE_BENCH(BM_Switch_LiteralMatch);
#endif

#if PTN_BENCH_ENABLE_SUITE_LITERAL_MATCH_128
PTN_REGISTER_STABLE_BENCH(BM_PatterniaPipe_LiteralMatch128);
PTN_REGISTER_STABLE_BENCH(BM_Switch_LiteralMatch128);
#endif

#if PTN_BENCH_ENABLE_SUITE_PACKET
PTN_REGISTER_STABLE_BENCH(BM_PatterniaPipe_PacketMixed);
PTN_REGISTER_STABLE_BENCH(BM_Switch_PacketMixed);
#endif

#if PTN_BENCH_ENABLE_SUITE_PACKET_HEAVY_BIND
PTN_REGISTER_STABLE_BENCH(BM_PatterniaPipe_PacketMixedHeavyBind);
PTN_REGISTER_STABLE_BENCH(BM_Switch_PacketMixedHeavyBind);
#endif
#elif PTN_BENCH_MODE == PTN_BENCH_MODE_PIPE_CHAIN
#if PTN_BENCH_ENABLE_SUITE_VARIANT
PTN_REGISTER_STABLE_BENCH(BM_Patternia_VariantMixed);
PTN_REGISTER_STABLE_BENCH(BM_PatterniaPipe_VariantMixed);
PTN_REGISTER_STABLE_BENCH(BM_Patternia_VariantAltHot);
PTN_REGISTER_STABLE_BENCH(BM_PatterniaPipe_VariantAltHot);
#endif

#if PTN_BENCH_ENABLE_SUITE_VARIANT_FASTPATH
PTN_REGISTER_STABLE_BENCH(BM_PatterniaPipe_VariantFastPathMixed);
PTN_REGISTER_STABLE_BENCH(BM_PatterniaPipe_VariantFastPathAltHot);
PTN_REGISTER_STABLE_BENCH(BM_PatterniaPipe_VariantAltIndexMixed);
PTN_REGISTER_STABLE_BENCH(BM_PatterniaPipe_VariantAltIndexAltHot);
PTN_REGISTER_STABLE_BENCH(BM_PatterniaPipe_VariantAltIndex32Mixed);
PTN_REGISTER_STABLE_BENCH(
    BM_PatterniaPipe_VariantAltIndex32MixedStatic);
PTN_REGISTER_STABLE_BENCH(BM_SwitchIndex_VariantAltIndex32Mixed);
PTN_REGISTER_STABLE_BENCH(BM_PatterniaPipe_VariantAltIndex32AltHot);
PTN_REGISTER_STABLE_BENCH(
    BM_PatterniaPipe_VariantAltIndex32AltHotStatic);
PTN_REGISTER_STABLE_BENCH(BM_SwitchIndex_VariantAltIndex32AltHot);
#endif

#if PTN_BENCH_ENABLE_SUITE_VARIANT_GUARDED
PTN_REGISTER_STABLE_BENCH(BM_Patternia_VariantMixedGuarded);
PTN_REGISTER_STABLE_BENCH(BM_PatterniaPipe_VariantMixedGuarded);
PTN_REGISTER_STABLE_BENCH(BM_Patternia_VariantAltHotGuarded);
PTN_REGISTER_STABLE_BENCH(BM_PatterniaPipe_VariantAltHotGuarded);
#endif

#if PTN_BENCH_ENABLE_SUITE_PROTOCOL_ROUTER
PTN_REGISTER_STABLE_BENCH(BM_Patternia_ProtocolRouter);
PTN_REGISTER_STABLE_BENCH(BM_PatterniaPipe_ProtocolRouter);
#endif

#if PTN_BENCH_ENABLE_SUITE_COMMAND_PARSER
PTN_REGISTER_STABLE_BENCH(BM_Patternia_CommandParser);
PTN_REGISTER_STABLE_BENCH(BM_PatterniaPipe_CommandParser);
#endif

#if PTN_BENCH_ENABLE_SUITE_LITERAL_MATCH
PTN_REGISTER_STABLE_BENCH(BM_Patternia_LiteralMatch);
PTN_REGISTER_STABLE_BENCH(BM_PatterniaPipe_LiteralMatch);
#endif

#if PTN_BENCH_ENABLE_SUITE_PACKET
PTN_REGISTER_STABLE_BENCH(BM_Patternia_PacketMixed);
PTN_REGISTER_STABLE_BENCH(BM_PatterniaPipe_PacketMixed);
#endif

#if PTN_BENCH_ENABLE_SUITE_PACKET_HEAVY_BIND
PTN_REGISTER_STABLE_BENCH(BM_Patternia_PacketMixedHeavyBind);
PTN_REGISTER_STABLE_BENCH(BM_PatterniaPipe_PacketMixedHeavyBind);
#endif
#else
#error                                                              \
    "Unsupported PTN_BENCH_MODE. Use PTN_BENCH_MODE_PIPE_STD or PTN_BENCH_MODE_PIPE_CHAIN."
#endif

#undef PTN_REGISTER_STABLE_BENCH
#undef PTN_SWITCH_BLOCK_16
#undef PTN_LIT_BLOCK_16
#undef PTN_SWITCH_CASE_128
#undef PTN_LIT_CASE_128
