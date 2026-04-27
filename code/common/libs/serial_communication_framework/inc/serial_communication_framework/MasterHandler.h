#ifndef MASTERPROTOCOLHANDLER_H
#define MASTERPROTOCOLHANDLER_H

#include <cstdint>
#include <span>

#include "command_interface.h"
#include "drivers/interfaces/BufferedSerialCommunicationInterface.h"
#include "drivers/interfaces/ClockInterface.h"
#include "serial_communication_framework/common.h"
#include "serial_communication_framework/packets.h"
#include "serial_communication_framework/serialize_deserialize.h"

namespace serial_communication_framework {

class MasterHandler {
public:
     MasterHandler(drivers::interfaces::BufferedSerialCommunicationInterface& communication_interface,
                   drivers::interfaces::ClockInterface&                       clock_interface);
    ~MasterHandler() = default;

    void init();

    // TODO tälle concepti tai static assert että on oikeesti commandi
    template <typename T_Command>
    [[nodiscard]] typename T_Command::Response sendCommandAndReceiveResponseBlocking(
        uint8_t receiver_id, typename T_Command::Request command_request) {
        RequestPacket request(receiver_id, T_Command::K_OP_CODE, command_request.serialize(command_staging_buffer));
        std::span<uint8_t> serialized_request = serializeRequest(request, tx_buffer_);
        communication_interface_.transmitBytes(serialized_request);

        startResponseTimeout();

        size_t rx_index             = 0;
        size_t expected_packet_size = ResponsePacket::K_PACKET_MAX_SIZE;

        while (rx_index < expected_packet_size) {
            if (responseHasTimedout()) {
                communication_statistics_.timed_out_packets++;

                typename T_Command::Response command_response;
                command_response.response_code = ResponseCode::timed_out;
                return command_response;
            }

            if (communication_interface_.getReceivedBytesAvailableAmount() > 0) {
                rx_buffer_[rx_index] = communication_interface_.readReceivedByte();
                rx_index++;
            }

            if (rx_index == ResponsePacket::K_HEADER_SIZE) {
                ResponsePacket::Header header = deSerializeResponseHeader(rx_buffer_);
                expected_packet_size          = header.payload_size + ResponsePacket::K_HEADER_WITH_PAYLOAD_CRC_SIZE;

                if (!responseHeaderHasValidCrc(header)) {
                    communication_statistics_.corrupted_packets_received++;

                    typename T_Command::Response command_response;
                    command_response.response_code = ResponseCode::corrupted;
                    return command_response;
                }
            }
        }

        ResponsePacket response = deSerializeResponse(rx_buffer_);
        communication_statistics_.total_packets_received++;

        if (!responsePayloadHasValidCrc(response)) {
            communication_statistics_.corrupted_packets_received++;

            typename T_Command::Response command_response;
            command_response.response_code = ResponseCode::corrupted;
            return command_response;
        }

        communication_statistics_.valid_packets_received++;

        // TODO Resetting tx buffer and rx buffer

        typename T_Command::Response command_response;
        command_response.response_code = static_cast<ResponseCode>(response.header.response_code);
        if (command_response.deserialize(response.payload) != commands::ParsingError::no_error) {
            // TODO HANDLE ERRORS
        }

        return command_response;
    }

    /*void sendCommandAndReceiveResponseASync(uint8_t receiver_id, uint8_t operation_code, std::span<uint8_t> payload,
                                            void(*cb));*/

    void run();

    [[nodiscard]] const CommunicationStatistics& getStatistics() const;

private:
    uint8_t tx_buffer_[RequestPacket::K_PACKET_MAX_SIZE]              = {};
    uint8_t rx_buffer_[ResponsePacket::K_PACKET_MAX_SIZE]             = {};
    uint8_t command_staging_buffer[RequestPacket::K_PAYLOAD_MAX_SIZE] = {};

    drivers::interfaces::BufferedSerialCommunicationInterface& communication_interface_;
    CommunicationStatistics                                    communication_statistics_;

    drivers::interfaces::ClockInterface& timeout_clock_;
    uint64_t                             response_timout_start_time_point_;

    // TODO: queue for requests and callbacks
    // AsyncCallBack next_response_cb_ = nullptr;

private:
    void startResponseTimeout();
    bool responseHasTimedout();
};

}  // namespace serial_communication_framework

#endif  // MASTERPROTOCOLHANDLER_H
