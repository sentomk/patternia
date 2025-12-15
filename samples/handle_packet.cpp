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

// Handlers
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

// Protocol parser
void parse_packet(const Packet &pkt) {

  // is_valid_payload predicate
  auto is_valid_payload = [&pkt](const std::vector<std::uint8_t> &payload) {
    return pkt.type == 0x02 && pkt.length == payload.size() &&
           (pkt.flags & FLAG_VALID);
  };

  // is_error_packet predicate
  auto is_error_packet = [](std::uint8_t                     type,
                            const std::vector<std::uint8_t> &payload) {
    return type == 0xFF && !payload.empty();
  };

  match(pkt)
      // PING
      .when(
          bind(
              has<&Packet::type,
                  &Packet::length>())[arg<0> == 0x01 && arg<1> == 0] >>
          [](auto &&...) { handle_ping(); })

      // DATA
      .when(
          bind(has<&Packet::payload>())[is_valid_payload] >>
          [](const std::vector<std::uint8_t> &payload) {
            handle_data(payload);
          })

      // ERROR
      .when(
          bind(has<&Packet::type, &Packet::payload>())[is_error_packet] >>
          [](std::uint8_t, const std::vector<std::uint8_t> &payload) {
            handle_error(payload[0]);
          })

      // FALLBACK
      .otherwise([] { reject_packet(); });
}

int main() {
  Packet ping{.type = 0x01, .length = 0, .flags = 0, .payload = {}};

  Packet data{
      .type = 0x02, .length = 3, .flags = FLAG_VALID, .payload = {1, 2, 3}};

  Packet error{.type = 0xFF, .length = 1, .flags = 0, .payload = {42}};

  Packet invalid{.type = 0x02, .length = 10, .flags = 0, .payload = {1}};

  parse_packet(ping);
  parse_packet(data);
  parse_packet(error);
  parse_packet(invalid);
}