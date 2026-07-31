// bench_scale.cpp — Variant dispatch scalability benchmarks.
//
// Measures Patternia vs std::visit vs switch(index) across 4, 8, 16,
// and 32 alternatives, using a realistic event-type hierarchy.

#include <benchmark/benchmark.h>

#include <cstddef>
#include <cstdint>
#include <variant>
#include <vector>

#include "ptn/patternia.hpp"

namespace {

  // --- Realistic event types (game engine / message broker) ---

  struct EvtConnect {
    uint32_t session_id;
  };
  struct EvtDisconnect {
    uint32_t session_id;
    uint16_t reason;
  };
  struct EvtMessage {
    uint32_t session_id;
    uint16_t type;
    uint16_t length;
  };
  struct EvtAck {
    uint32_t session_id;
    uint32_t seq;
  };
  struct EvtPing {
    uint64_t timestamp;
  };
  struct EvtPong {
    uint64_t timestamp;
  };
  struct EvtError {
    uint16_t code;
    bool     fatal;
  };
  struct EvtTimeout {
    uint32_t session_id;
    uint32_t ms;
  };
  struct EvtSubscribe {
    uint32_t session_id;
    uint16_t channel;
  };
  struct EvtUnsubscribe {
    uint32_t session_id;
    uint16_t channel;
  };
  struct EvtPublish {
    uint16_t channel;
    uint16_t length;
  };
  struct EvtHeartbeat {
    uint64_t timestamp;
    uint32_t session_id;
  };
  struct EvtResize {
    uint32_t session_id;
    uint16_t width;
    uint16_t height;
  };
  struct EvtInput {
    uint16_t key;
    uint16_t modifiers;
  };
  struct EvtFocus {
    uint32_t session_id;
    bool     gained;
  };
  struct EvtRender {
    uint64_t frame;
    uint16_t pass;
  };
  struct EvtAudio {
    uint16_t stream;
    uint16_t volume;
  };
  struct EvtConfig {
    uint16_t key;
    int32_t  value;
  };
  struct EvtShutdown {
    uint8_t mode;
    bool    force;
  };
  struct EvtLog {
    uint16_t level;
    uint16_t facility;
  };
  struct EvtMetrics {
    uint32_t count;
    uint32_t latency_us;
  };
  struct EvtNotify {
    uint32_t session_id;
    uint16_t priority;
  };
  struct EvtStream {
    uint32_t session_id;
    uint16_t stream_id;
    bool     open;
  };
  struct EvtSync {
    uint64_t timestamp;
    uint32_t session_id;
  };
  struct EvtQuery {
    uint16_t type;
    uint16_t limit;
    uint32_t session_id;
  };
  struct EvtResponse {
    uint16_t status;
    uint16_t length;
  };
  struct EvtAuth {
    uint32_t session_id;
    uint16_t method;
    bool     success;
  };
  struct EvtEncrypt {
    uint16_t algo;
    uint16_t key_id;
  };
  struct EvtDecrypt {
    uint16_t algo;
    uint16_t key_id;
  };
  struct EvtCompress {
    uint16_t algo;
    uint16_t level;
  };
  struct EvtCache {
    uint16_t op;
    uint32_t size;
  };
  struct EvtRetry {
    uint32_t session_id;
    uint8_t  attempt;
  };

  using Evt4 = std::
      variant<EvtConnect, EvtDisconnect, EvtMessage, EvtAck>;
  using Evt8  = std::variant<EvtConnect,
                             EvtDisconnect,
                             EvtMessage,
                             EvtAck,
                             EvtPing,
                             EvtPong,
                             EvtError,
                             EvtTimeout>;
  using Evt16 = std::variant<EvtConnect,
                             EvtDisconnect,
                             EvtMessage,
                             EvtAck,
                             EvtPing,
                             EvtPong,
                             EvtError,
                             EvtTimeout,
                             EvtSubscribe,
                             EvtUnsubscribe,
                             EvtPublish,
                             EvtHeartbeat,
                             EvtResize,
                             EvtInput,
                             EvtFocus,
                             EvtRender>;
  using Evt32 = std::variant<EvtConnect,
                             EvtDisconnect,
                             EvtMessage,
                             EvtAck,
                             EvtPing,
                             EvtPong,
                             EvtError,
                             EvtTimeout,
                             EvtSubscribe,
                             EvtUnsubscribe,
                             EvtPublish,
                             EvtHeartbeat,
                             EvtResize,
                             EvtInput,
                             EvtFocus,
                             EvtRender,
                             EvtAudio,
                             EvtConfig,
                             EvtShutdown,
                             EvtLog,
                             EvtMetrics,
                             EvtNotify,
                             EvtStream,
                             EvtSync,
                             EvtQuery,
                             EvtResponse,
                             EvtAuth,
                             EvtEncrypt,
                             EvtDecrypt,
                             EvtCompress,
                             EvtCache,
                             EvtRetry>;

  // ---- Patternia dispatchers ----

  int scaleN_ptn4(const Evt4 &e) {
    using namespace ptn;
    using ptn::pat::is;
    return match(e)
           | PTN_ON(is<EvtConnect>() >> 1,
                    is<EvtDisconnect>() >> 2,
                    is<EvtMessage>() >> 3,
                    is<EvtAck>() >> 4,
                    _ >> 0);
  }

  int scaleN_ptn8(const Evt8 &e) {
    using namespace ptn;
    using ptn::pat::is;
    return match(e)
           | PTN_ON(is<EvtConnect>() >> 1,
                    is<EvtDisconnect>() >> 2,
                    is<EvtMessage>() >> 3,
                    is<EvtAck>() >> 4,
                    is<EvtPing>() >> 5,
                    is<EvtPong>() >> 6,
                    is<EvtError>() >> 7,
                    is<EvtTimeout>() >> 8,
                    _ >> 0);
  }

  int scaleN_ptn16(const Evt16 &e) {
    using namespace ptn;
    using ptn::pat::is;
    return match(e)
           | PTN_ON(is<EvtConnect>() >> 1,
                    is<EvtDisconnect>() >> 2,
                    is<EvtMessage>() >> 3,
                    is<EvtAck>() >> 4,
                    is<EvtPing>() >> 5,
                    is<EvtPong>() >> 6,
                    is<EvtError>() >> 7,
                    is<EvtTimeout>() >> 8,
                    is<EvtSubscribe>() >> 9,
                    is<EvtUnsubscribe>() >> 10,
                    is<EvtPublish>() >> 11,
                    is<EvtHeartbeat>() >> 12,
                    is<EvtResize>() >> 13,
                    is<EvtInput>() >> 14,
                    is<EvtFocus>() >> 15,
                    is<EvtRender>() >> 16,
                    _ >> 0);
  }

  int scaleN_ptn32(const Evt32 &e) {
    using namespace ptn;
    using ptn::pat::is;
    return match(e)
           | PTN_ON(is<EvtConnect>() >> 1,
                    is<EvtDisconnect>() >> 2,
                    is<EvtMessage>() >> 3,
                    is<EvtAck>() >> 4,
                    is<EvtPing>() >> 5,
                    is<EvtPong>() >> 6,
                    is<EvtError>() >> 7,
                    is<EvtTimeout>() >> 8,
                    is<EvtSubscribe>() >> 9,
                    is<EvtUnsubscribe>() >> 10,
                    is<EvtPublish>() >> 11,
                    is<EvtHeartbeat>() >> 12,
                    is<EvtResize>() >> 13,
                    is<EvtInput>() >> 14,
                    is<EvtFocus>() >> 15,
                    is<EvtRender>() >> 16,
                    is<EvtAudio>() >> 17,
                    is<EvtConfig>() >> 18,
                    is<EvtShutdown>() >> 19,
                    is<EvtLog>() >> 20,
                    is<EvtMetrics>() >> 21,
                    is<EvtNotify>() >> 22,
                    is<EvtStream>() >> 23,
                    is<EvtSync>() >> 24,
                    is<EvtQuery>() >> 25,
                    is<EvtResponse>() >> 26,
                    is<EvtAuth>() >> 27,
                    is<EvtEncrypt>() >> 28,
                    is<EvtDecrypt>() >> 29,
                    is<EvtCompress>() >> 30,
                    is<EvtCache>() >> 31,
                    is<EvtRetry>() >> 32,
                    _ >> 0);
  }

  // ---- StdVisit dispatchers ----

  int scaleN_visit4(const Evt4 &e) {
    return std::visit(
        [](const auto &ev) -> int {
          using T = std::decay_t<decltype(ev)>;
          if constexpr (std::is_same_v<T, EvtConnect>)
            return 1;
          else if constexpr (std::is_same_v<T, EvtDisconnect>)
            return 2;
          else if constexpr (std::is_same_v<T, EvtMessage>)
            return 3;
          else if constexpr (std::is_same_v<T, EvtAck>)
            return 4;
          else
            return 0;
        },
        e);
  }

  int scaleN_visit8(const Evt8 &e) {
    return std::visit(
        [](const auto &ev) -> int {
          using T = std::decay_t<decltype(ev)>;
          if constexpr (std::is_same_v<T, EvtConnect>)
            return 1;
          else if constexpr (std::is_same_v<T, EvtDisconnect>)
            return 2;
          else if constexpr (std::is_same_v<T, EvtMessage>)
            return 3;
          else if constexpr (std::is_same_v<T, EvtAck>)
            return 4;
          else if constexpr (std::is_same_v<T, EvtPing>)
            return 5;
          else if constexpr (std::is_same_v<T, EvtPong>)
            return 6;
          else if constexpr (std::is_same_v<T, EvtError>)
            return 7;
          else if constexpr (std::is_same_v<T, EvtTimeout>)
            return 8;
          else
            return 0;
        },
        e);
  }

  int scaleN_visit16(const Evt16 &e) {
    return std::visit(
        [](const auto &ev) -> int {
          using T = std::decay_t<decltype(ev)>;
          if constexpr (std::is_same_v<T, EvtConnect>)
            return 1;
          else if constexpr (std::is_same_v<T, EvtDisconnect>)
            return 2;
          else if constexpr (std::is_same_v<T, EvtMessage>)
            return 3;
          else if constexpr (std::is_same_v<T, EvtAck>)
            return 4;
          else if constexpr (std::is_same_v<T, EvtPing>)
            return 5;
          else if constexpr (std::is_same_v<T, EvtPong>)
            return 6;
          else if constexpr (std::is_same_v<T, EvtError>)
            return 7;
          else if constexpr (std::is_same_v<T, EvtTimeout>)
            return 8;
          else if constexpr (std::is_same_v<T, EvtSubscribe>)
            return 9;
          else if constexpr (std::is_same_v<T, EvtUnsubscribe>)
            return 10;
          else if constexpr (std::is_same_v<T, EvtPublish>)
            return 11;
          else if constexpr (std::is_same_v<T, EvtHeartbeat>)
            return 12;
          else if constexpr (std::is_same_v<T, EvtResize>)
            return 13;
          else if constexpr (std::is_same_v<T, EvtInput>)
            return 14;
          else if constexpr (std::is_same_v<T, EvtFocus>)
            return 15;
          else if constexpr (std::is_same_v<T, EvtRender>)
            return 16;
          else
            return 0;
        },
        e);
  }

  int scaleN_visit32(const Evt32 &e) {
    return std::visit(
        [](const auto &ev) -> int {
          using T = std::decay_t<decltype(ev)>;
          if constexpr (std::is_same_v<T, EvtConnect>)
            return 1;
          else if constexpr (std::is_same_v<T, EvtDisconnect>)
            return 2;
          else if constexpr (std::is_same_v<T, EvtMessage>)
            return 3;
          else if constexpr (std::is_same_v<T, EvtAck>)
            return 4;
          else if constexpr (std::is_same_v<T, EvtPing>)
            return 5;
          else if constexpr (std::is_same_v<T, EvtPong>)
            return 6;
          else if constexpr (std::is_same_v<T, EvtError>)
            return 7;
          else if constexpr (std::is_same_v<T, EvtTimeout>)
            return 8;
          else if constexpr (std::is_same_v<T, EvtSubscribe>)
            return 9;
          else if constexpr (std::is_same_v<T, EvtUnsubscribe>)
            return 10;
          else if constexpr (std::is_same_v<T, EvtPublish>)
            return 11;
          else if constexpr (std::is_same_v<T, EvtHeartbeat>)
            return 12;
          else if constexpr (std::is_same_v<T, EvtResize>)
            return 13;
          else if constexpr (std::is_same_v<T, EvtInput>)
            return 14;
          else if constexpr (std::is_same_v<T, EvtFocus>)
            return 15;
          else if constexpr (std::is_same_v<T, EvtRender>)
            return 16;
          else if constexpr (std::is_same_v<T, EvtAudio>)
            return 17;
          else if constexpr (std::is_same_v<T, EvtConfig>)
            return 18;
          else if constexpr (std::is_same_v<T, EvtShutdown>)
            return 19;
          else if constexpr (std::is_same_v<T, EvtLog>)
            return 20;
          else if constexpr (std::is_same_v<T, EvtMetrics>)
            return 21;
          else if constexpr (std::is_same_v<T, EvtNotify>)
            return 22;
          else if constexpr (std::is_same_v<T, EvtStream>)
            return 23;
          else if constexpr (std::is_same_v<T, EvtSync>)
            return 24;
          else if constexpr (std::is_same_v<T, EvtQuery>)
            return 25;
          else if constexpr (std::is_same_v<T, EvtResponse>)
            return 26;
          else if constexpr (std::is_same_v<T, EvtAuth>)
            return 27;
          else if constexpr (std::is_same_v<T, EvtEncrypt>)
            return 28;
          else if constexpr (std::is_same_v<T, EvtDecrypt>)
            return 29;
          else if constexpr (std::is_same_v<T, EvtCompress>)
            return 30;
          else if constexpr (std::is_same_v<T, EvtCache>)
            return 31;
          else if constexpr (std::is_same_v<T, EvtRetry>)
            return 32;
          else
            return 0;
        },
        e);
  }

  // ---- SwitchIndex dispatchers ----

  int scaleN_switch4(const Evt4 &e) {
    switch (e.index()) {
    case 0:
      return 1;
    case 1:
      return 2;
    case 2:
      return 3;
    case 3:
      return 4;
    default:
      return 0;
    }
  }

  int scaleN_switch8(const Evt8 &e) {
    switch (e.index()) {
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
    default:
      return 0;
    }
  }

  int scaleN_switch16(const Evt16 &e) {
    switch (e.index()) {
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
    default:
      return 0;
    }
  }

  int scaleN_switch32(const Evt32 &e) {
    switch (e.index()) {
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

  // ---- Workloads ----

  const std::vector<Evt4> &wl4() {
    static const std::vector<Evt4> data = {
        Evt4{std::in_place_index<0>, EvtConnect{1}},
        Evt4{std::in_place_index<1>, EvtDisconnect{1, 100}},
        Evt4{std::in_place_index<2>, EvtMessage{1, 200, 10}},
        Evt4{std::in_place_index<3>, EvtAck{1, 0}},
        Evt4{std::in_place_index<0>, EvtConnect{2}},
        Evt4{std::in_place_index<2>, EvtMessage{2, 201, 20}},
        Evt4{std::in_place_index<1>, EvtDisconnect{2, 101}},
        Evt4{std::in_place_index<3>, EvtAck{2, 1}},
    };
    return data;
  }

  const std::vector<Evt8> &wl8() {
    static const std::vector<Evt8> data = {
        Evt8{std::in_place_index<0>, EvtConnect{1}},
        Evt8{std::in_place_index<1>, EvtDisconnect{1, 100}},
        Evt8{std::in_place_index<2>, EvtMessage{1, 200, 10}},
        Evt8{std::in_place_index<3>, EvtAck{1, 0}},
        Evt8{std::in_place_index<4>, EvtPing{1000}},
        Evt8{std::in_place_index<5>, EvtPong{1001}},
        Evt8{std::in_place_index<6>, EvtError{500, false}},
        Evt8{std::in_place_index<7>, EvtTimeout{1, 3000}},
        Evt8{std::in_place_index<0>, EvtConnect{2}},
        Evt8{std::in_place_index<4>, EvtPing{1002}},
        Evt8{std::in_place_index<2>, EvtMessage{2, 201, 20}},
        Evt8{std::in_place_index<6>, EvtError{501, true}},
        Evt8{std::in_place_index<3>, EvtAck{2, 1}},
        Evt8{std::in_place_index<7>, EvtTimeout{2, 5000}},
        Evt8{std::in_place_index<1>, EvtDisconnect{2, 102}},
        Evt8{std::in_place_index<5>, EvtPong{1003}},
    };
    return data;
  }

  const std::vector<Evt16> &wl16() {
    static const std::vector<Evt16> data = {
        Evt16{std::in_place_index<0>, EvtConnect{1}},
        Evt16{std::in_place_index<3>, EvtAck{1, 0}},
        Evt16{std::in_place_index<6>, EvtError{500, false}},
        Evt16{std::in_place_index<9>, EvtUnsubscribe{1, 5}},
        Evt16{std::in_place_index<12>, EvtResize{1, 1920, 1080}},
        Evt16{std::in_place_index<1>, EvtDisconnect{1, 100}},
        Evt16{std::in_place_index<4>, EvtPing{1000}},
        Evt16{std::in_place_index<7>, EvtTimeout{1, 3000}},
        Evt16{std::in_place_index<10>, EvtPublish{5, 100}},
        Evt16{std::in_place_index<13>, EvtInput{65, 0}},
        Evt16{std::in_place_index<2>, EvtMessage{1, 200, 10}},
        Evt16{std::in_place_index<5>, EvtPong{1001}},
        Evt16{std::in_place_index<8>, EvtSubscribe{1, 3}},
        Evt16{std::in_place_index<11>, EvtHeartbeat{2000, 1}},
        Evt16{std::in_place_index<14>, EvtFocus{1, true}},
        Evt16{std::in_place_index<15>, EvtRender{1, 0}},
    };
    return data;
  }

  const std::vector<Evt32> &wl32() {
    static const std::vector<Evt32> data = {
        Evt32{std::in_place_index<0>, EvtConnect{1}},
        Evt32{std::in_place_index<15>, EvtRender{1, 0}},
        Evt32{std::in_place_index<7>, EvtTimeout{1, 3000}},
        Evt32{std::in_place_index<23>, EvtSync{1000, 1}},
        Evt32{std::in_place_index<3>, EvtAck{1, 0}},
        Evt32{std::in_place_index<19>, EvtLog{2, 10}},
        Evt32{std::in_place_index<11>, EvtHeartbeat{2000, 1}},
        Evt32{std::in_place_index<27>, EvtEncrypt{1, 100}},
        Evt32{std::in_place_index<6>, EvtError{500, false}},
        Evt32{std::in_place_index<31>, EvtRetry{1, 1}},
        Evt32{std::in_place_index<2>, EvtMessage{1, 200, 10}},
        Evt32{std::in_place_index<16>, EvtAudio{1, 80}},
        Evt32{std::in_place_index<9>, EvtUnsubscribe{1, 5}},
        Evt32{std::in_place_index<25>, EvtResponse{200, 50}},
        Evt32{std::in_place_index<1>, EvtDisconnect{1, 100}},
        Evt32{std::in_place_index<20>, EvtMetrics{100, 500}},
        Evt32{std::in_place_index<12>, EvtResize{1, 1920, 1080}},
        Evt32{std::in_place_index<28>, EvtDecrypt{1, 100}},
        Evt32{std::in_place_index<4>, EvtPing{1000}},
        Evt32{std::in_place_index<17>, EvtConfig{1, 42}},
        Evt32{std::in_place_index<8>, EvtSubscribe{1, 3}},
        Evt32{std::in_place_index<24>, EvtQuery{1, 100, 1}},
        Evt32{std::in_place_index<10>, EvtPublish{5, 100}},
        Evt32{std::in_place_index<30>, EvtCache{1, 1024}},
        Evt32{std::in_place_index<5>, EvtPong{1001}},
        Evt32{std::in_place_index<21>, EvtNotify{1, 1}},
        Evt32{std::in_place_index<13>, EvtInput{65, 0}},
        Evt32{std::in_place_index<26>, EvtAuth{1, 0, true}},
        Evt32{std::in_place_index<14>, EvtFocus{1, true}},
        Evt32{std::in_place_index<22>, EvtStream{1, 0, true}},
        Evt32{std::in_place_index<18>, EvtShutdown{0, false}},
        Evt32{std::in_place_index<29>, EvtCompress{1, 6}},
    };
    return data;
  }

  template <typename T, typename F>
  static void
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

#define DEF_BENCH(Name, Wl, Fn)                                     \
  static void BM_##Name(benchmark::State &state) {                  \
    run_wl(state, Wl(), Fn);                                        \
  }                                                                 \
  BENCHMARK(BM_##Name)                                              \
      ->Unit(benchmark::kNanosecond)                                \
      ->MinTime(0.5)                                                \
      ->Repetitions(20)                                             \
      ->ReportAggregatesOnly(true)

  // ScaleN/4
  DEF_BENCH(PatterniaPipe_ScaleN4, wl4, scaleN_ptn4);
  DEF_BENCH(StdVisit_ScaleN4, wl4, scaleN_visit4);
  DEF_BENCH(SwitchIndex_ScaleN4, wl4, scaleN_switch4);

  // ScaleN/8
  DEF_BENCH(PatterniaPipe_ScaleN8, wl8, scaleN_ptn8);
  DEF_BENCH(StdVisit_ScaleN8, wl8, scaleN_visit8);
  DEF_BENCH(SwitchIndex_ScaleN8, wl8, scaleN_switch8);

  // ScaleN/16
  DEF_BENCH(PatterniaPipe_ScaleN16, wl16, scaleN_ptn16);
  DEF_BENCH(StdVisit_ScaleN16, wl16, scaleN_visit16);
  DEF_BENCH(SwitchIndex_ScaleN16, wl16, scaleN_switch16);

  // ScaleN/32
  DEF_BENCH(PatterniaPipe_ScaleN32, wl32, scaleN_ptn32);
  DEF_BENCH(StdVisit_ScaleN32, wl32, scaleN_visit32);
  DEF_BENCH(SwitchIndex_ScaleN32, wl32, scaleN_switch32);

} // namespace
