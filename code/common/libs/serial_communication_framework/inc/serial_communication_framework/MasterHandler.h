#ifndef MASTERPROTOCOLHANDLER_H
#define MASTERPROTOCOLHANDLER_H

#include <cstdint>
#include <span>

#include "drivers/interfaces/SerialBufferedCommunicationInterface.h"
#include "serial_communication_framework/common.h"

namespace serial_communication_framework {

using AsyncCallBack = void (*)(ResponseData);

class MasterHandler {
public:
     MasterHandler(std::span<uint8_t> tx_buffer, std::span<uint8_t> rx_buffer,
                   drivers::interfaces::SerialBufferedCommunicationInterface& communication_interface);
    ~MasterHandler() = default;

    ResponseData sendRequestAndReceiveResponseBlocking(uint8_t receiver_id, uint8_t operation_code,
                                                       std::span<uint8_t> payload);
    void sendRequestAndReceiveResponseASync(uint8_t receiver_id, uint8_t operation_code, std::span<uint8_t> payload,
                                            AsyncCallBack cb);

    void run();

private:
    std::span<uint8_t> tx_buffer_;
    std::span<uint8_t> rx_buffer_;

    drivers::interfaces::SerialBufferedCommunicationInterface& communication_interface_;

    // TODO: queue for requests and callbacks
    AsyncCallBack next_response_cb_ = nullptr;
};

}  // namespace serial_communication_framework

#endif  // MASTERPROTOCOLHANDLER_H
