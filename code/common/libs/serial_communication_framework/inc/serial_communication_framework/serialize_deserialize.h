#ifndef PARSER_H
#define PARSER_H

#include "serial_communication_framework/packets.h"

namespace serial_communication_framework {

bool responseHasValidCrc(ResponsePacket packet);
bool requestHasValidCrc(RequestPacket packet);

[[nodiscard]] ResponsePacket deSerializeResponse(std::span<uint8_t> data);
[[nodiscard]] RequestPacket  deSerializeRequest(std::span<uint8_t> data);

[[nodiscard]] std::span<uint8_t> serializeResponse(ResponsePacket resp, std::span<uint8_t> target_buffer);
[[nodiscard]] std::span<uint8_t> serializeRequest(RequestPacket req, std::span<uint8_t> target_buffer);

[[nodiscard]] ResponsePacket::Header deSerializeResponseHeader(std::span<uint8_t> data);
[[nodiscard]] RequestPacket::Header  deSerializeRequestHeader(std::span<uint8_t> data);

}  // namespace serial_communication_framework

#endif  // PARSER_H
