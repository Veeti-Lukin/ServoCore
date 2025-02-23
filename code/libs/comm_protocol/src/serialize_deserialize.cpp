#include "comm_protocol/serialize_deserialize.h"

#include <cstring>

namespace comm_protocol {

bool packetHasValidCrc(protocol_definitions::Packet packet) {
    // TODO implement
    return true;
}

protocol_definitions::Packet deSerializePacket(std::span<uint8_t> data) {
    protocol_definitions::Packet packet;
    // TODO assert data.size_bytes >= K_PACKET_MIN_SIZE

    packet.receiver_id    = data[0];
    packet.operation_code = data[1];
    packet.payload_length = data[2];
    packet.crc            = data[3];

    packet.payload        = data.subspan(protocol_definitions::K_HEADER_SIZE, packet.payload_length);
    /*
    uint8_t* payload_start = data.data() + protocol_definitions::K_HEADER_SIZE;
    std::memcpy(packet.payload.data(), payload_start, packet.payload_length);
    */
    return packet;
}

std::span<uint8_t> serializePacket(protocol_definitions::Packet packet, std::span<uint8_t> buffer) {
    buffer[0]              = packet.receiver_id;
    buffer[1]              = packet.operation_code;
    buffer[2]              = packet.payload_length;
    buffer[3]              = packet.crc;

    uint8_t* payload_start = buffer.data() + protocol_definitions::K_HEADER_SIZE;
    std::memcpy(payload_start, packet.payload.data(), packet.payload_length);

    return buffer.subspan(0, protocol_definitions::K_HEADER_SIZE + packet.payload_length);
}

protocol_definitions::Packet deSerializePacketHeader(std::span<uint8_t> data) {
    protocol_definitions::Packet packet;

    packet.receiver_id    = data[0];
    packet.operation_code = data[1];
    packet.payload_length = data[2];
    packet.crc            = data[3];

    return packet;
}

}  // namespace comm_protocol