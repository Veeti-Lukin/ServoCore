#include "serial_communication_framework/MasterHandler.h"

#include "assert/assert.h"
#include "serial_communication_framework/packets.h"
#include "serial_communication_framework/serialize_deserialize.h"
#include "utils/StaticList.h"

namespace serial_communication_framework {

MasterHandler::MasterHandler(drivers::interfaces::BufferedSerialCommunicationInterface& communication_interface,
                             drivers::interfaces::ClockInterface&                       clock_interface)
    : timeout_clock_(clock_interface), communication_interface_(communication_interface) {
    ASSERT_WITH_MESSAGE(std::span<uint8_t>(tx_buffer_).size_bytes() >= RequestPacket::K_PACKET_MAX_SIZE,
                        "Too small tx_buffer");
    ASSERT_WITH_MESSAGE(std::span<uint8_t>(rx_buffer_).size_bytes() >= ResponsePacket::K_PACKET_MAX_SIZE,
                        "Too small rx_buffer");
}

void MasterHandler::init() { /* TODO SET THE SERIAL COMMUNICATION SETTINGS */
}

ResponseData MasterHandler::sendRequestAndReceiveResponseBlocking(
    uint8_t receiver_id, uint8_t operation_code,
    utils::StaticList<uint8_t, RequestPacket::K_PAYLOAD_MAX_SIZE> payload) {
    RequestPacket      request(receiver_id, operation_code, payload);
    std::span<uint8_t> serialized_request = serializeRequest(request, tx_buffer_);
    communication_interface_.transmitBytes(serialized_request);

    startResponseTimeout();

    size_t rx_index             = 0;
    size_t expected_packet_size = ResponsePacket::K_PACKET_MAX_SIZE;

    while (rx_index < expected_packet_size) {
        if (responseHasTimedout()) {
            communication_statistics_.timed_out_packets++;
            return {ResponseCode::timed_out, {}};
        }

        if (communication_interface_.getReceivedBytesAvailableAmount() > 0) {
            rx_buffer_[rx_index] = communication_interface_.readReceivedByte();
            rx_index++;
        }

        if (rx_index == ResponsePacket::K_HEADER_SIZE) {
            ResponsePacket::Header header = deSerializeResponseHeader(rx_buffer_);
            expected_packet_size          = header.payload_size + ResponsePacket::K_HEADER_WITH_CRC_SIZE;
        }
    }

    ResponsePacket response = deSerializeResponse(rx_buffer_);
    communication_statistics_.total_packets_received++;

    if (responseHasValidCrc(response) == false) {
        communication_statistics_.corrupted_packets_received++;
        return ResponseData(ResponseCode::corrupted, {});
    }

    communication_statistics_.valid_packets_received++;

    // TODO Resetting tx buffer and rx buffer

    return ResponseData{static_cast<ResponseCode>(response.header.response_code),
                        utils::StaticList<uint8_t, ResponsePacket::K_PAYLOAD_MAX_SIZE>(response.payload)};
}

void MasterHandler::sendRequestAndReceiveResponseASync(uint8_t receiver_id, uint8_t operation_code,
                                                       std::span<uint8_t> payload, AsyncCallBack cb) {
    // TODO implement
}
void MasterHandler::run() {
    // TODO implement
}

const CommunicationStatistics& MasterHandler::getStatistics() const { return communication_statistics_; }

void MasterHandler::startResponseTimeout() { response_timout_start_time_point_ = timeout_clock_.uptimeMilliseconds(); }

bool MasterHandler::responseHasTimedout() {
#ifdef SERVO_CORE_DISABLE_SERIAL_COMMUNICATION_FRAMEWORK_TIMEOUTS
    return false;
#endif

    const uint64_t now = timeout_clock_.uptimeMilliseconds();
    return (now - response_timout_start_time_point_) > K_MASTER_TIMEOUT_MS;
}

}  // namespace serial_communication_framework
