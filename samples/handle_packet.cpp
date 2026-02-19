#include "ptn/patternia.hpp"
#include <iostream>
#include <cstdint>

using namespace ptn;

constexpr std::uint8_t FLAG_VALID = 0x01;

struct Packet {
  std::uint8_t              type;
  std::uint16_t             length;
  std::uint8_t              flags;
  std::vector<std::uint8_t> payload;
};

// Packet handlers.
void handle_ping() {
  std::cout << "PING\n";
}

void handle_data(const std::vector<std::uint8_t> &data) {
  std::cout << "DATA, size=" << data.size() << "\n";
}

void handle_error(std::uint8_t code) {
  std::cout << "ERROR, code=" << int(code) << "\n";
}

void reject_packet() {
  std::cout << "REJECT\n";
}

// Protocol parser.
void parse_packet(const Packet &pkt) {

  // Predicate for valid data payload packets.
  auto is_valid_payload = [&pkt](const std::vector<std::uint8_t> &payload) {
    return pkt.type == 0x02 && pkt.length == payload.size() &&
           (pkt.flags & FLAG_VALID);
  };

  // Predicate for error packets carrying payload bytes.
  auto is_error_packet = [](std::uint8_t                     type,
                            const std::vector<std::uint8_t> &payload) {
    return type == 0xFF && !payload.empty();
  };

  match(pkt)
      // Handles ping packets.
      .when(
          bind(
              has<&Packet::type,
                  &Packet::length>())[arg<0> == 0x01 && arg<1> == 0] >>
          [](auto &&...) { handle_ping(); })

      // Handles data packets.
      .when(
          bind(has<&Packet::payload>())[is_valid_payload] >>
          [](const std::vector<std::uint8_t> &payload) {
            handle_data(payload);
          })

      // Handles error packets.
      .when(
          bind(has<&Packet::type, &Packet::payload>())[is_error_packet] >>
          [](std::uint8_t, const std::vector<std::uint8_t> &payload) {
            handle_error(payload[0]);
          })

      // Handles unmatched packets.
      .otherwise([] { reject_packet(); });
}

int main() {
  Packet ping{
      0x01, // type
      0,    // length
      0,    // flags
      {}    // payload
  };

  Packet data{
      0x02,       // type
      3,          // length
      FLAG_VALID, // flags
      {1, 2, 3}   // payload
  };

  Packet error{
      0xFF, // type
      1,    // length
      0,    // flags
      {42}  // payload
  };

  Packet invalid{
      0x02, // type
      10,   // length
      0,    // flags
      {1}   // payload
  };

  parse_packet(ping);
  parse_packet(data);
  parse_packet(error);
  parse_packet(invalid);
}
