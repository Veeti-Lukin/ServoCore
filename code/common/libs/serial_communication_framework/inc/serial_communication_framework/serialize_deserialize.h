#ifndef PARSER_H
#define PARSER_H

#include "serial_communication_framework/packets.h"

namespace serial_communication_framework {

bool responsePayloadHasValidCrc(const ResponsePacket& packet);
bool requestPayloadHasValidCrc(const RequestPacket& packet);
bool responseHeaderHasValidCrc(const ResponsePacket::Header& header);
bool requestHeaderHasValidCrc(const RequestPacket::Header& header);

[[nodiscard]] ResponsePacket deSerializeResponse(std::span<uint8_t> data);
[[nodiscard]] RequestPacket  deSerializeRequest(std::span<uint8_t> data);

[[nodiscard]] std::span<uint8_t> serializeResponse(const ResponsePacket& resp, std::span<uint8_t> target_buffer);
[[nodiscard]] std::span<uint8_t> serializeRequest(const RequestPacket& req, std::span<uint8_t> target_buffer);

[[nodiscard]] ResponsePacket::Header deSerializeResponseHeader(std::span<uint8_t> data);
[[nodiscard]] std::span<uint8_t>     serializeResponseHeader(const ResponsePacket::Header& resp_header,
                                                             std::span<uint8_t>            target_buffer);

[[nodiscard]] RequestPacket::Header deSerializeRequestHeader(std::span<uint8_t> data);
[[nodiscard]] std::span<uint8_t>    serializeRequestHeader(const RequestPacket::Header& req_header,
                                                           std::span<uint8_t>           target_buffer);

}  // namespace serial_communication_framework

#endif  // PARSER_H
