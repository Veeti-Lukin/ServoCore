#ifndef PARSER_H
#define PARSER_H

#include "comm_protocol/protocol_definition.h"

namespace comm_protocol {

bool packetHasValidCrc(protocol_definitions::Packet packet);

[[nodiscard]] protocol_definitions::Packet deSerializePacket(std::span<uint8_t> data);
[[nodiscard]] std::span<uint8_t> serializePacket(protocol_definitions::Packet packet, std::span<uint8_t> buffer);

[[nodiscard]] protocol_definitions::Packet deSerializePacketHeader(std::span<uint8_t> data);

}  // namespace comm_protocol

#endif  // PARSER_H
