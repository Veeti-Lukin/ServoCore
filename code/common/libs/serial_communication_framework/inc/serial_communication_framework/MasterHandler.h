#ifndef MASTERPROTOCOLHANDLER_H
#define MASTERPROTOCOLHANDLER_H

#include <cstdint>
#include <span>

#include "drivers/interfaces/BufferedSerialCommunicationInterface.h"
#include "serial_communication_framework/common.h"
#include "serial_communication_framework/packets.h"

namespace serial_communication_framework {

using AsyncCallBack = void (*)(ResponseData);

class MasterHandler {
public:
    explicit MasterHandler(drivers::interfaces::BufferedSerialCommunicationInterface& communication_interface);
    ~        MasterHandler() = default;

    [[nodiscard]] ResponseData sendRequestAndReceiveResponseBlocking(uint8_t receiver_id, uint8_t operation_code,
                                                                     std::span<uint8_t> payload);
    void sendRequestAndReceiveResponseASync(uint8_t receiver_id, uint8_t operation_code, std::span<uint8_t> payload,
                                            AsyncCallBack cb);

    void run();

private:
    uint8_t tx_buffer_[ResponsePacket::K_PACKET_MAX_SIZE] = {};
    uint8_t rx_buffer_[RequestPacket::K_PACKET_MAX_SIZE]  = {};

    drivers::interfaces::BufferedSerialCommunicationInterface& communication_interface_;

    // TODO: queue for requests and callbacks
    AsyncCallBack next_response_cb_ = nullptr;
};

}  // namespace serial_communication_framework

#endif  // MASTERPROTOCOLHANDLER_H
