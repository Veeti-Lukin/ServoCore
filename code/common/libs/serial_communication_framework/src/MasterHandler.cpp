#include "serial_communication_framework/MasterHandler.h"

#include "assert/assert.h"
#include "serial_communication_framework/packets.h"
#include "serial_communication_framework/serialize_deserialize.h"

namespace serial_communication_framework {
MasterHandler::MasterHandler(std::span<uint8_t> tx_buffer, std::span<uint8_t> rx_buffer,
                             drivers::interfaces::SerialBufferedCommunicationInterface& communication_interface)
    : tx_buffer_(tx_buffer), rx_buffer_(rx_buffer), communication_interface_(communication_interface) {
    ASSERT_WITH_MESSAGE(tx_buffer_.size_bytes() >= ResponsePacket::K_PACKET_MAX_SIZE, "Too small tx_buffer");
    ASSERT_WITH_MESSAGE(rx_buffer_.size_bytes() >= RequestPacket::K_PACKET_MAX_SIZE, "Too small rx_buffer");
}

ResponseData MasterHandler::sendRequestAndReceiveResponseBlocking(uint8_t receiver_id, uint8_t operation_code,
                                                                  std::span<uint8_t> payload) {
    RequestPacket      request(receiver_id, operation_code, payload);
    std::span<uint8_t> serialized_request = serializeRequest(request, tx_buffer_);
    communication_interface_.transmitBytes(serialized_request);

    size_t rx_index             = 0;
    size_t expected_packet_size = ResponsePacket::K_PACKET_MAX_SIZE;

    while (rx_index < expected_packet_size) {
        if (communication_interface_.getReceivedBytesAvailableAmount() > 0) {
            rx_buffer_[rx_index] = communication_interface_.readReceivedByte();
            rx_index++;
        }

        if (rx_index == ResponsePacket::K_HEADER_SIZE) {
            ResponsePacket::Header header = deSerializeResponseHeader(rx_buffer_);
            expected_packet_size          = header.payload_size + RequestPacket::K_HEADER_WITH_CRC_SIZE;
        }
    }

    ResponsePacket response = deSerializeResponse(rx_buffer_);

    if (responseHasValidCrc(response) == false) {
        // TODO log statistics
        return ResponseData(ResponseCode::corrupted, {});
    }

    // TODO Resetting tx buffer and rx buffer

    return ResponseData{static_cast<ResponseCode>(response.header.response_code), response.payload};
}

void MasterHandler::sendRequestAndReceiveResponseASync(uint8_t receiver_id, uint8_t operation_code,
                                                       std::span<uint8_t> payload, AsyncCallBack cb) {
    // TODO implement
}
void MasterHandler::run() {
    // TODO implement
}

}  // namespace serial_communication_framework
