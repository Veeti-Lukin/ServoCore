#include "serial_communication_framework/SlaveHandler.h"

#include "assert/assert.h"
#include "serial_communication_framework/packets.h"
#include "serial_communication_framework/serialize_deserialize.h"

namespace serial_communication_framework {

SlaveHandler::SlaveHandler(std::span<OperationCodeHandlerInfo>                        op_code_handler_buffer,
                           drivers::interfaces::BufferedSerialCommunicationInterface& communication_interface,
                           drivers::interfaces::ClockInterface& clock_interface, uint8_t device_id)
    : communication_interface_(communication_interface),
      device_id_(device_id),
      timeout_clock_(clock_interface),
      op_code_handlers_(op_code_handler_buffer) {
    ASSERT_WITH_MESSAGE(std::span<uint8_t>(tx_buffer_).size_bytes() >= ResponsePacket::K_PACKET_MAX_SIZE,
                        "Too small tx_buffer");
    ASSERT_WITH_MESSAGE(std::span<uint8_t>(rx_buffer_).size_bytes() >= RequestPacket::K_PACKET_MAX_SIZE,
                        "Too small rx_buffer");
}

void SlaveHandler::init() { /* TODO SET THE SERIAL COMMUNICATION SETTINGS */
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

        startResponseTimeout();

        RequestPacket packet = deSerializeRequest(rx_buffer_);

        // TODO which way around should this be check the crc first or the id
        // if id then if tha packet is still corrupted and the id field is faulty this device might conflict with
        // the device the packet was meant for if it gets the packet correctly if crc first and the packet is
        // corrupted the the packet might be for some other device and the if it gets the packet correctly same
        // issue happens

        // This way it is really unlikely that even if the packet is corrupted te id of the packet would be device
        // id

        // Check if the packet is for this device or not
        // If not, do not do anything with the packet
        if (packet.header.receiver_id != device_id_) {
            return;
        }

        // only increment this after te id checking
        communication_statistics_.total_packets_received++;

        if (requestHasValidCrc(packet) == false) {
            communication_statistics_.corrupted_packets_received++;
            ResponsePacket     response(static_cast<uint8_t>(ResponseCode::corrupted), {});
            std::span<uint8_t> serialized_response = serializeResponse(response, tx_buffer_);

            if (responseHasTimedout()) {
                communication_statistics_.timed_out_packets++;
                // Do not answer if the timeout has happened on slave side and let the master run to timeout
                return;
            }

            communication_interface_.transmitBytes(serialized_response);
            return;
        }
        communication_statistics_.valid_packets_received++;

        OperationCodeHandler op_code_handler = getOpcodeHandler(packet.header.operation_code);

        ResponseData response_data;
        if (op_code_handler == nullptr) {
            response_data = {ResponseCode::unknown_operation_code, {}};
        } else {
            response_data = op_code_handler(packet.payload);
        }

        ResponsePacket     response(static_cast<uint8_t>(response_data.response_code), response_data.response_data);
        std::span<uint8_t> serialized_response = serializeResponse(response, tx_buffer_);

        if (responseHasTimedout()) {
            communication_statistics_.timed_out_packets++;
            // Do not answer if the timeout has happened on slave side and let the master run to timeout
            return;
        }
        communication_interface_.transmitBytes(serialized_response);

        // TODO Resetting tx buffer and rx buffer
    }
}

const CommunicationStatistics& SlaveHandler::getCommunicationStatistics() const { return communication_statistics_; }

OperationCodeHandler SlaveHandler::getOpcodeHandler(uint8_t op_code) {
    for (OperationCodeHandlerInfo op_code_info : op_code_handlers_) {
        if (op_code_info.operation_code == op_code) {
            return op_code_info.handler;
        }
    }

    return nullptr;
}

void SlaveHandler::startResponseTimeout() { response_timout_start_time_point_ = timeout_clock_.uptimeMilliseconds(); }

bool SlaveHandler::responseHasTimedout() {
#ifdef SERVO_CORE_DISABLE_SERIAL_COMMUNICATION_FRAMEWORK_TIMEOUTS
    return false;
#endif

    const uint64_t now = timeout_clock_.uptimeMilliseconds();
    return (now - response_timout_start_time_point_) > K_SLAVE_TIMEOUT_MS;
}

}  // namespace serial_communication_framework