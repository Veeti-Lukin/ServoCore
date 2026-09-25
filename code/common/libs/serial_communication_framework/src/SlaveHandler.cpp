#include "serial_communication_framework/SlaveHandler.h"

#include <span>

#include "assert/assert.h"
#include "serial_communication_framework/packets.h"
#include "serial_communication_framework/serialize_deserialize.h"

namespace serial_communication_framework {

SlaveHandler::SlaveHandler(drivers::interfaces::BufferedSerialCommunicationInterface& communication_interface,
                           drivers::interfaces::ClockInterface& clock_interface, uint8_t device_id)
    : communication_interface_(communication_interface), device_id_(device_id), timeout_clock_(clock_interface) {
    ASSERT_WITH_MESSAGE(std::span<uint8_t>(tx_buffer_).size_bytes() >= ResponsePacket::K_PACKET_MAX_SIZE,
                        "Too small tx_buffer");
    ASSERT_WITH_MESSAGE(std::span<uint8_t>(rx_buffer_).size_bytes() >= RequestPacket::K_PACKET_MAX_SIZE,
                        "Too small rx_buffer");
}

void SlaveHandler::init() { /* TODO SET THE SERIAL COMMUNICATION SETTINGS */ }

void SlaveHandler::run() {
    static size_t rx_index             = 0;
    static size_t expected_packet_size = RequestPacket::K_PACKET_MAX_SIZE;

    if (communication_interface_.getReceivedBytesAvailableAmount() > 0) {
        rx_buffer_[rx_index] = communication_interface_.readReceivedByte();
        rx_index++;
    }

    if (rx_index == RequestPacket::K_HEADER_SIZE) {
        RequestPacket::Header header = deSerializeRequestHeader(rx_buffer_);

        if (!requestHeaderHasValidCrc(header)) {
            // The receiver id is part of the header that just failed, so this one can never be
            // attributed to a device
            communication_statistics_.all_requests.received++;
            communication_statistics_.all_requests.corrupted++;

            // restore index to default
            rx_index = 0;

            // A corrupted packet is not answered at all, the master is left to run to its timeout
            return;
        }

        expected_packet_size = header.payload_size + RequestPacket::K_HEADER_WITH_PAYLOAD_CRC_SIZE;
        return;
    }

    // TODO SIZE OFF BY 1 indexing error?
    if (rx_index == expected_packet_size) {
        // restore to defaults
        rx_index             = 0;
        expected_packet_size = RequestPacket::K_PACKET_MAX_SIZE;

        startResponseTimeout();

        RequestPacket packet = deSerializeRequest(rx_buffer_);

        // The header crc already passed, so the receiver id can be trusted for the statistics even when
        // the payload turns out to be corrupt
        const bool for_this_device = packet.header.receiver_id == device_id_;

        communication_statistics_.all_requests.received++;
        if (for_this_device) communication_statistics_.requests_for_this_device.received++;

        if (!requestPayloadHasValidCrc(packet)) {
            communication_statistics_.all_requests.corrupted++;
            if (for_this_device) communication_statistics_.requests_for_this_device.corrupted++;

            // A corrupted packet is not answered at all, the master is left to run to its timeout
            return;
        }

        communication_statistics_.all_requests.valid++;
        if (for_this_device) communication_statistics_.requests_for_this_device.valid++;

        // Only a request meant for this device is acted on
        if (!for_this_device) return;

        AdapterFunc adapter_func = command_handlers_[packet.header.operation_code];

        AdapterFuncResponse adapter_func_response;
        if (adapter_func == nullptr) {
            adapter_func_response = {ResponseCode::unknown_operation_code, {}};
        } else {
            adapter_func_response = adapter_func(this, packet.payload);
        }

        ResponsePacket     response(static_cast<uint8_t>(adapter_func_response.response_code),
                                    adapter_func_response.response_data);
        std::span<uint8_t> serialized_response = serializeResponse(response, tx_buffer_);

        if (responseHasTimedout()) {
            communication_statistics_.dropped_late_answers++;
            // Do not answer if the timeout has happened on slave side and let the master run to timeout
            return;
        }
        communication_interface_.transmitBytes(serialized_response);

        // TODO Resetting tx buffer and rx buffer
    }
}

const SlaveCommunicationStatistics& SlaveHandler::getCommunicationStatistics() const { return communication_statistics_; }

void SlaveHandler::startResponseTimeout() { response_timout_start_time_point_ = timeout_clock_.uptimeMilliseconds(); }

bool SlaveHandler::responseHasTimedout() {
#ifdef SERVO_CORE_DISABLE_SERIAL_COMMUNICATION_FRAMEWORK_TIMEOUTS
    return false;
#endif

    const uint64_t now = timeout_clock_.uptimeMilliseconds();
    return (now - response_timout_start_time_point_) > K_SLAVE_TIMEOUT_MS;
}

}  // namespace serial_communication_framework