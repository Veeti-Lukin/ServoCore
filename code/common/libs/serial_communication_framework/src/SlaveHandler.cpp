#include "serial_communication_framework/SlaveHandler.h"

#include "assert/assert.h"
#include "serial_communication_framework/packets.h"
#include "serial_communication_framework/serialize_deserialize.h"

namespace serial_communication_framework {

SlaveHandler::SlaveHandler(std::span<uint8_t> tx_buffer, std::span<uint8_t> rx_buffer,
                           std::span<OperationCodeHandlerInfo>                        op_code_handler_buffer,
                           drivers::interfaces::SerialBufferedCommunicationInterface& communication_interface,
                           uint8_t                                                    device_id)
    : tx_buffer_(tx_buffer),
      rx_buffer_(rx_buffer),
      communication_interface_(communication_interface),
      op_code_handlers_(op_code_handler_buffer),
      device_id_(device_id) {
    ASSERT_WITH_MESSAGE(tx_buffer_.size_bytes() >= ResponsePacket::K_PACKET_MAX_SIZE, "Too small tx_buffer");
    ASSERT_WITH_MESSAGE(rx_buffer_.size_bytes() >= RequestPacket::K_PACKET_MAX_SIZE, "Too small rx_buffer");
}

void SlaveHandler::registerHandler(uint8_t op_code, OperationCodeHandler handler) {
    ASSERT_WITH_MESSAGE(op_code_handlers_.size() > op_code_handler_registering_index_,
                        "Op code handler registering index out of bounds. Too small buffer");
    op_code_handlers_[op_code_handler_registering_index_] = {op_code, handler};
    op_code_handler_registering_index_++;
}

void SlaveHandler::run() {
    static size_t rx_index             = 0;
    static size_t expected_packet_size = RequestPacket::K_PACKET_MAX_SIZE;

    if (communication_interface_.getReceivedBytesAvailableAmount() > 0) {
        rx_buffer_[rx_index] = communication_interface_.readReceivedByte();
        rx_index++;
    }

    if (rx_index == RequestPacket::K_HEADER_SIZE) {
        RequestPacket::Header header = deSerializeRequestHeader(rx_buffer_);
        expected_packet_size         = header.payload_size + RequestPacket::K_HEADER_WITH_CRC_SIZE;
        return;
    }

    // TODO SIZE OFF BY 1 indexing error?
    if (rx_index == expected_packet_size) {
        // restore to defaults
        rx_index             = 0;
        expected_packet_size = RequestPacket::K_PACKET_MAX_SIZE;

        RequestPacket packet = deSerializeRequest(rx_buffer_);

        if (requestHasValidCrc(packet) == false) {
            // TODO log statistics
            ResponsePacket     response(static_cast<uint8_t>(ResponseCode::corrupted), {});
            std::span<uint8_t> serialized_response = serializeResponse(response, tx_buffer_);
            communication_interface_.transmitBytes(serialized_response);
            return;
        }

        // Check if the packet is for this device or not
        // If not, do not do anything with the packet
        if (packet.header.receiver_id != device_id_) {
            return;
        }

        OperationCodeHandler op_code_handler = getOpcodeHandler(packet.header.operation_code);

        ResponseData response_data;
        if (op_code_handler == nullptr) {
            response_data = {ResponseCode::unknown_operation_code, {}};
        } else {
            response_data = op_code_handler(packet.payload);
        }

        ResponsePacket     response(static_cast<uint8_t>(response_data.response_code), response_data.response_data);
        std::span<uint8_t> serialized_response = serializeResponse(response, tx_buffer_);
        communication_interface_.transmitBytes(serialized_response);

        // TODO Resetting tx buffer and rx buffer
    }
}

OperationCodeHandler SlaveHandler::getOpcodeHandler(uint8_t op_code) {
    for (OperationCodeHandlerInfo op_code_info : op_code_handlers_) {
        if (op_code_info.operation_code == op_code) {
            return op_code_info.handler;
        }
    }

    return nullptr;
}

}  // namespace serial_communication_framework