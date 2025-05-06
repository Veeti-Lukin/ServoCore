#ifndef MASTERPROTOCOLHANDLER_H
#define MASTERPROTOCOLHANDLER_H

#include <cstdint>
#include <span>

#include "drivers/interfaces/BufferedSerialCommunicationInterface.h"
#include "drivers/interfaces/ClockInterface.h"
#include "serial_communication_framework/common.h"
#include "serial_communication_framework/packets.h"
#include "utils/StaticList.h"

namespace serial_communication_framework {

using AsyncCallBack = void (*)(ResponseData);

class MasterHandler {
public:
     MasterHandler(drivers::interfaces::BufferedSerialCommunicationInterface& communication_interface,
                   drivers::interfaces::ClockInterface&                       clock_interface);
    ~MasterHandler() = default;

    void init();

    [[nodiscard]] ResponseData sendRequestAndReceiveResponseBlocking(
        uint8_t receiver_id, uint8_t operation_code,
        utils::StaticList<uint8_t, RequestPacket::K_PAYLOAD_MAX_SIZE> payload);
    void sendRequestAndReceiveResponseASync(uint8_t receiver_id, uint8_t operation_code, std::span<uint8_t> payload,
                                            AsyncCallBack cb);

    void run();

    [[nodiscard]] const CommunicationStatistics& getStatistics() const;

private:
    uint8_t tx_buffer_[RequestPacket::K_PACKET_MAX_SIZE]  = {};
    uint8_t rx_buffer_[ResponsePacket::K_PACKET_MAX_SIZE] = {};

    drivers::interfaces::BufferedSerialCommunicationInterface& communication_interface_;
    CommunicationStatistics                                    communication_statistics_;

    drivers::interfaces::ClockInterface& timeout_clock_;
    uint64_t                             response_timout_start_time_point_;

    // TODO: queue for requests and callbacks
    AsyncCallBack next_response_cb_ = nullptr;

    void startResponseTimeout();
    bool responseHasTimedout();
};

}  // namespace serial_communication_framework

#endif  // MASTERPROTOCOLHANDLER_H
