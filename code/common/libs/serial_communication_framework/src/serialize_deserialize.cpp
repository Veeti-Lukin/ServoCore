#include "serial_communication_framework/serialize_deserialize.h"

#include <cstring>

#include "assert/assert.h"

namespace serial_communication_framework {

bool responseHasValidCrc(ResponsePacket packet) {
    // TODO implement
    return true;
}

bool requestHasValidCrc(RequestPacket packet) {
    // TODO implement
    return true;
}

ResponsePacket deSerializeResponse(std::span<uint8_t> data) {
    ASSERT(data.size_bytes() >= ResponsePacket::K_PACKET_MIN_SIZE);

    ResponsePacket resp;
    resp.header = deSerializeResponseHeader(data.subspan(0, ResponsePacket::K_HEADER_SIZE));

    ASSERT(data.size_bytes() >= ResponsePacket::K_HEADER_WITH_CRC_SIZE + resp.header.payload_size);
    resp.payload = data.subspan(ResponsePacket::K_PAYLOAD_START_OFFSET, resp.header.payload_size);

    return resp;
}

RequestPacket deSerializeRequest(std::span<uint8_t> data) {
    ASSERT(data.size_bytes() >= RequestPacket::K_PACKET_MIN_SIZE);

    RequestPacket req;
    req.header = deSerializeRequestHeader(data.subspan(0, RequestPacket::K_HEADER_SIZE));

    ASSERT(data.size_bytes() >= RequestPacket::K_HEADER_WITH_CRC_SIZE + req.header.payload_size);
    req.payload = data.subspan(RequestPacket::K_PAYLOAD_START_OFFSET, req.header.payload_size);

    return req;
}

std::span<uint8_t> serializeResponse(ResponsePacket resp, std::span<uint8_t> target_buffer) {
    ASSERT(target_buffer.size_bytes() >= RequestPacket::K_PAYLOAD_MAX_SIZE);
    // Check that the de serialization still since we are assuming that these values are only one byte long
    static_assert(sizeof(ResponsePacket::Header::response_code) == 1);
    static_assert(sizeof(ResponsePacket::Header::payload_size) == 1);
    static_assert(sizeof(ResponsePacket::Header::header_crc) == 1);
    static_assert(sizeof(ResponsePacket::payload_crc) == 1);

    target_buffer[0]       = resp.header.response_code;
    target_buffer[1]       = resp.header.payload_size;
    target_buffer[2]       = resp.header.header_crc;

    target_buffer[3]       = resp.payload_crc;

    uint8_t* payload_start = target_buffer.data() + ResponsePacket::K_PAYLOAD_START_OFFSET;
    std::memcpy(payload_start, resp.payload.data(), resp.header.payload_size);

    return target_buffer.subspan(0, ResponsePacket::K_HEADER_WITH_CRC_SIZE + resp.header.payload_size);
}

std::span<uint8_t> serializeRequest(RequestPacket req, std::span<uint8_t> target_buffer) {
    ASSERT(target_buffer.size_bytes() >= RequestPacket::K_PAYLOAD_MAX_SIZE);
    // Check that the de serialization still works since we are assuming that these values are only one byte long
    static_assert(sizeof(RequestPacket::Header::receiver_id) == 1);
    static_assert(sizeof(RequestPacket::Header::operation_code) == 1);
    static_assert(sizeof(RequestPacket::Header::payload_size) == 1);
    static_assert(sizeof(RequestPacket::Header::header_crc) == 1);
    static_assert(sizeof(RequestPacket::payload_crc) == 1);

    target_buffer[0]       = req.header.receiver_id;
    target_buffer[1]       = req.header.operation_code;
    target_buffer[2]       = req.header.payload_size;
    target_buffer[3]       = req.header.header_crc;

    target_buffer[4]       = req.payload_crc;

    uint8_t* payload_start = target_buffer.data() + RequestPacket::K_PAYLOAD_START_OFFSET;
    std::memcpy(payload_start, req.payload.data(), req.header.payload_size);

    return target_buffer.subspan(0, RequestPacket::K_HEADER_WITH_CRC_SIZE + req.header.payload_size);
}

ResponsePacket::Header deSerializeResponseHeader(std::span<uint8_t> data) {
    ASSERT(data.size_bytes() >= ResponsePacket::K_HEADER_SIZE);

    ResponsePacket::Header resp_header;
    // Check that the de serialization still since we are assuming that these values are only one byte long
    static_assert(sizeof(ResponsePacket::Header::response_code) == 1);
    static_assert(sizeof(ResponsePacket::Header::payload_size) == 1);
    static_assert(sizeof(ResponsePacket::Header::header_crc) == 1);

    resp_header.response_code = data[0];
    resp_header.payload_size  = data[1];
    resp_header.header_crc    = data[2];

    return resp_header;
}

RequestPacket::Header deSerializeRequestHeader(std::span<uint8_t> data) {
    ASSERT(data.size_bytes() >= RequestPacket::K_HEADER_SIZE);

    RequestPacket::Header req_header;
    // Check that the de serialization still works since we are assuming that these values are only one byte long
    static_assert(sizeof(RequestPacket::Header::receiver_id) == 1);
    static_assert(sizeof(RequestPacket::Header::operation_code) == 1);
    static_assert(sizeof(RequestPacket::Header::payload_size) == 1);
    static_assert(sizeof(RequestPacket::Header::header_crc) == 1);

    req_header.receiver_id    = data[0];
    req_header.operation_code = data[1];
    req_header.payload_size   = data[2];
    req_header.header_crc     = data[3];

    return req_header;
}

}  // namespace serial_communication_framework