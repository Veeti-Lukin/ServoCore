#ifndef PROTOCOL_DEFINITION_H
#define PROTOCOL_DEFINITION_H

#include <cstdint>
#include <limits>
#include <span>

namespace comm_protocol::protocol_definitions {

struct Packet {
    uint8_t            receiver_id;
    uint8_t            operation_code;
    uint8_t            payload_length;
    uint8_t            crc;
    std::span<uint8_t> payload;
};

// TODO CONSTRUCT PACKET FUNCTION THAT ONLY TAKES THE VALUABLE INFO IN? (NO CRC NO PAYLOAD SIZE)

constexpr size_t K_PAYLOAD_MAX_SIZE = std::numeric_limits<decltype(Packet::payload_length)>::max();
constexpr size_t K_HEADER_SIZE =
    sizeof(Packet::receiver_id) + sizeof(Packet::operation_code) + sizeof(Packet::payload_length) + sizeof(Packet::crc);
constexpr size_t K_PACKET_MAX_SIZE                    = K_HEADER_SIZE + K_PAYLOAD_MAX_SIZE;
constexpr size_t K_PACKET_MIN_SIZE                    = K_HEADER_SIZE;

constexpr uint8_t K_RESERVED_MASTER_ID                = 0;

constexpr uint8_t K_RESPONSE_TO_MASTER_OPERATION_CODE = 0xff;

}  // namespace comm_protocol::protocol_definitions

#endif  // PROTOCOL_DEFINITION_H
