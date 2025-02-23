#ifndef PARSER_H
#define PARSER_H

#include "comm_protocol/packets.h"

namespace comm_protocol {

bool responseHasValidCrc(ResponsePacket packet);
bool requestHasValidCrc(RequestPacket packet);

[[nodiscard]] ResponsePacket deSerializeResponse(std::span<uint8_t> data);
[[nodiscard]] RequestPacket  deSerializeRequest(std::span<uint8_t> data);

[[nodiscard]] std::span<uint8_t> serializeResponse(ResponsePacket resp, std::span<uint8_t> target_buffer);
[[nodiscard]] std::span<uint8_t> serializeRequest(RequestPacket req, std::span<uint8_t> target_buffer);

[[nodiscard]] ResponsePacket::Header deSerializeResponseHeader(std::span<uint8_t> data);
[[nodiscard]] RequestPacket::Header  deSerializeRequestHeader(std::span<uint8_t> data);

}  // namespace comm_protocol

#endif  // PARSER_H
