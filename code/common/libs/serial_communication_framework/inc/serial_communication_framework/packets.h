#ifndef PROTOCOL_DEFINITION_H
#define PROTOCOL_DEFINITION_H

#include <bit>
#include <cstdint>
#include <limits>
#include <span>

// The wire format places multi-byte fields in little-endian order and serdes uses memcpy directly
// against the host representation — no byte swapping. If master and slave disagree on endianness
// (e.g. host is big-endian but the firmware target is little-endian), every multi-byte field on the
// wire would be silently misinterpreted. Both currently supported targets (RP2040, x86 Windows/Linux)
// are little-endian, so we lock that in here rather than pay the cost of byte-swap logic on every field.
// TODO After adding this, check what where we can refactor code to use memcpy directly now
static_assert(std::endian::native == std::endian::little,
              "serial_communication_framework format assumes little-endian host. "
              "Big-endian support is not implemented.");

namespace serial_communication_framework {

struct RequestPacket {
    struct Header {
        uint8_t receiver_id    = 0;
        uint8_t operation_code = 0;
        uint8_t payload_size   = 0;
        uint8_t header_crc     = 0;
    };

    Header             header                  = {};
    uint8_t            payload_crc             = 0;
    std::span<uint8_t> payload                 = {};

    static constexpr size_t K_PAYLOAD_MAX_SIZE = std::numeric_limits<decltype(Header::payload_size)>::max();
    static constexpr size_t K_HEADER_SIZE      = sizeof(Header::receiver_id) + sizeof(Header::operation_code) +
                                            sizeof(Header::payload_size) + sizeof(Header::header_crc);
    static constexpr size_t K_HEADER_SIZE_WITHOUT_CRC      = K_HEADER_SIZE - sizeof(Header::header_crc);

    static constexpr size_t K_HEADER_WITH_PAYLOAD_CRC_SIZE = K_HEADER_SIZE + sizeof(payload_crc);

    static constexpr size_t K_PACKET_MAX_SIZE              = K_HEADER_SIZE + K_PAYLOAD_MAX_SIZE + sizeof(payload_crc);
    static constexpr size_t K_PACKET_MIN_SIZE              = K_HEADER_SIZE + sizeof(payload_crc);

    static constexpr size_t K_PAYLOAD_START_OFFSET         = K_HEADER_SIZE + sizeof(payload_crc);

    RequestPacket()                                        = default;
    RequestPacket(uint8_t receiver_id, uint8_t operation_code, std::span<uint8_t> payload)
        : header{
              .receiver_id    = receiver_id,
              .operation_code = operation_code,
              .payload_size   = static_cast<uint8_t>(payload.size_bytes()),
              .header_crc = 0},  // will be assigned when packet is serialized, since the data might change before that
          payload_crc(0),        // will be assigned when packet is serialized, since the data might change before that
          payload(payload) {}
};

struct ResponsePacket {
    struct Header {
        uint8_t response_code = 0;
        uint8_t payload_size  = 0;
        uint8_t header_crc    = 0;
    };

    Header             header                  = {};
    uint8_t            payload_crc             = 0;
    std::span<uint8_t> payload                 = {};

    static constexpr size_t K_PAYLOAD_MAX_SIZE = std::numeric_limits<decltype(Header::payload_size)>::max();
    static constexpr size_t K_HEADER_SIZE =
        sizeof(Header::response_code) + sizeof(Header::payload_size) + sizeof(Header::header_crc);
    static constexpr size_t K_HEADER_SIZE_WITHOUT_CRC      = K_HEADER_SIZE - sizeof(Header::header_crc);
    static constexpr size_t K_HEADER_WITH_PAYLOAD_CRC_SIZE = K_HEADER_SIZE + sizeof(payload_crc);

    static constexpr size_t K_PACKET_MAX_SIZE              = K_HEADER_SIZE + K_PAYLOAD_MAX_SIZE + sizeof(payload_crc);
    static constexpr size_t K_PACKET_MIN_SIZE              = K_HEADER_SIZE + sizeof(payload_crc);

    static constexpr size_t K_PAYLOAD_START_OFFSET         = K_HEADER_SIZE + sizeof(payload_crc);

    ResponsePacket()                                       = default;
    ResponsePacket(uint8_t response_code, std::span<uint8_t> payload)
        : header{
              .response_code = response_code,
              .payload_size  = static_cast<uint8_t>(payload.size_bytes()),
              .header_crc = 0},  // will be assigned when packet is serialized, since the data might change before that
          payload_crc(0),        // will be assigned when packet is serialized, since the data might change before that
          payload(payload) {}
};

}  // namespace serial_communication_framework

#endif  // PROTOCOL_DEFINITION_H
