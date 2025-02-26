#include "comm_protocol/serialize_deserialize.h"

#include <cstring>

#include "assert/assert.h"

namespace comm_protocol {

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

    resp.header.response_code = data[0];
    resp.header.payload_size  = data[1];
    resp.crc                  = data[2];

    ASSERT(data.size_bytes() >= ResponsePacket::K_HEADER_WITH_CRC_SIZE + resp.header.payload_size);

    resp.payload = data.subspan(ResponsePacket::K_PAYLOAD_START_OFFSET, resp.header.payload_size);

    return resp;
}

RequestPacket deSerializeRequest(std::span<uint8_t> data) {
    ASSERT(data.size_bytes() >= RequestPacket::K_PACKET_MIN_SIZE);

    RequestPacket req;

    req.header.receiver_id    = data[0];
    req.header.operation_code = data[1];
    req.header.payload_size   = data[2];
    req.crc                   = data[3];

    ASSERT(data.size_bytes() >= RequestPacket::K_HEADER_WITH_CRC_SIZE + req.header.payload_size);

    req.payload = data.subspan(RequestPacket::K_PAYLOAD_START_OFFSET, req.header.payload_size);
    return req;
}

std::span<uint8_t> serializeResponse(ResponsePacket resp, std::span<uint8_t> target_buffer) {
    target_buffer[0]       = resp.header.response_code;
    target_buffer[1]       = resp.header.payload_size;
    target_buffer[2]       = resp.crc;

    uint8_t* payload_start = target_buffer.data() + ResponsePacket::K_PAYLOAD_START_OFFSET;
    std::memcpy(payload_start, resp.payload.data(), resp.header.payload_size);

    return target_buffer.subspan(0, ResponsePacket::K_HEADER_WITH_CRC_SIZE + resp.header.payload_size);
}

std::span<uint8_t> serializeRequest(RequestPacket req, std::span<uint8_t> target_buffer) {
    target_buffer[0]       = req.header.receiver_id;
    target_buffer[1]       = req.header.operation_code;
    target_buffer[2]       = req.header.payload_size;
    target_buffer[3]       = req.crc;

    uint8_t* payload_start = target_buffer.data() + RequestPacket::K_PAYLOAD_START_OFFSET;
    std::memcpy(payload_start, req.payload.data(), req.header.payload_size);

    return target_buffer.subspan(0, RequestPacket::K_HEADER_WITH_CRC_SIZE + req.header.payload_size);
}

ResponsePacket::Header deSerializeResponseHeader(std::span<uint8_t> data) {
    ASSERT(data.size_bytes() >= ResponsePacket::K_HEADER_SIZE);

    ResponsePacket::Header resp_header;
    resp_header.response_code = data[0];
    resp_header.payload_size  = data[1];

    return resp_header;
}

RequestPacket::Header deSerializeRequestHeader(std::span<uint8_t> data) {
    ASSERT(data.size_bytes() >= RequestPacket::K_HEADER_SIZE);

    RequestPacket::Header req_header;
    req_header.receiver_id    = data[0];
    req_header.operation_code = data[1];
    req_header.payload_size   = data[2];

    return req_header;
}

}  // namespace comm_protocol