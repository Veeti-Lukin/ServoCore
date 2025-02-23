#include "comm_protocol/ProtocolHandler.h"

#include "comm_protocol/protocol_definition.h"
#include "comm_protocol/serialize_deserialize.h"

namespace comm_protocol {

ProtocolHandler::ProtocolHandler(std::span<uint8_t> tx_buffer, std::span<uint8_t> rx_buffer,
                                 std::span<OperationCodeHandlerInfo>                        op_code_handler_buffer,
                                 drivers::interfaces::SerialBufferedCommunicationInterface& communication_interface)
    : tx_buffer_(tx_buffer),
      rx_buffer_(rx_buffer),
      op_code_handlers_(op_code_handler_buffer),
      communication_interface_(communication_interface) {
    // TODO assert tx_buffer.size_bytes() == K_MAX_PACKET_SIZE
    // TODO assert rx_buffer.size_bytes() == K_MAX_PACKET_SIZE
}

void ProtocolHandler::registerHandler(uint8_t op_code, OperationCodeHandler handler) {
    // assert registering index not bigger than buffer size
    op_code_handlers_[op_code_handler_registering_index_] = {op_code, handler};
    op_code_handler_registering_index_++;
}

void ProtocolHandler::update() {
    static size_t rx_index             = 0;
    static size_t expected_packet_size = protocol_definitions::K_PACKET_MAX_SIZE;

    if (communication_interface_.getReceivedBytesAvailableAmount() > 0) {
        rx_buffer_[rx_index] = communication_interface_.readReceivedByte();
        rx_index++;
    }

    // TODO SIZE OFF BY 1 indexing error?
    if (rx_index == protocol_definitions::K_HEADER_SIZE) {
        protocol_definitions::Packet header = deSerializePacketHeader(rx_buffer_);
        expected_packet_size                = header.payload_length + protocol_definitions::K_HEADER_SIZE;
        return;
    }

    // TODO SIZE OFF BY 1 indexing error?
    if (rx_index == expected_packet_size) {
        // restore to defaults
        rx_index                            = 0;
        expected_packet_size                = protocol_definitions::K_PACKET_MAX_SIZE;

        protocol_definitions::Packet packet = deSerializePacket(rx_buffer_);
        if (packetHasValidCrc(packet) == false) {
            // TODO do what?
        }

        OperationCodeHandler op_code_handler = getOpcodeHandler(packet.operation_code);

        ResponseData response_data;
        if (op_code_handler == nullptr) {
            response_data = {ResponseCode::unknown_operation_code, {}};
        } else {
            response_data = getOpcodeHandler(packet.operation_code)(packet.payload);
        }

        protocol_definitions::Packet response_packet     = constructResponsePacket(response_data);
        std::span<uint8_t>           serialized_response = serializePacket(response_packet, tx_buffer_);
        communication_interface_.transmitBytes(serialized_response);

        // TODO Resetting tx buffer and rx buffer
    }
}

OperationCodeHandler ProtocolHandler::getOpcodeHandler(uint8_t op_code) {
    for (OperationCodeHandlerInfo op_code_info : op_code_handlers_) {
        if (op_code_info.operation_code == op_code) {
            return op_code_info.handler;
        }
    }

    return nullptr;
}

protocol_definitions::Packet ProtocolHandler::constructResponsePacket(ResponseData response_data) {
    return {.receiver_id    = protocol_definitions::K_RESERVED_MASTER_ID,
            .operation_code = protocol_definitions::K_RESPONSE_TO_MASTER_OPERATION_CODE,
            .payload_length = static_cast<uint8_t>(response_data.response_data.size_bytes()),
            .crc            = 0,
            .payload        = response_data.response_data};
    // TODO RESPONSE CODE EI HÄNDLÄTÄ OLLENKAAN
}

}  // namespace comm_protocol