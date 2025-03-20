#ifndef PROTOCOLHANDLER_H
#define PROTOCOLHANDLER_H

#include <cstdint>
#include <span>

#include "drivers/interfaces/BufferedSerialCommunicationInterface.h"
#include "serial_communication_framework/common.h"
#include "serial_communication_framework/packets.h"

namespace serial_communication_framework {

using OperationCodeHandler = ResponseData (*)(std::span<std::uint8_t> request_data);

struct OperationCodeHandlerInfo {
    uint8_t              operation_code;
    OperationCodeHandler handler;
};

class SlaveHandler {
public:
    explicit SlaveHandler(std::span<OperationCodeHandlerInfo>                        op_code_handler_buffer,
                          drivers::interfaces::BufferedSerialCommunicationInterface& communication_interface,
                          uint8_t                                                    device_id);
    ~        SlaveHandler() = default;

    void registerHandler(uint8_t op_code, OperationCodeHandler handler);

    void run();

private:
    OperationCodeHandler getOpcodeHandler(uint8_t op_code);

    uint8_t tx_buffer_[ResponsePacket::K_PACKET_MAX_SIZE] = {};
    uint8_t rx_buffer_[RequestPacket::K_PACKET_MAX_SIZE]  = {};

    drivers::interfaces::BufferedSerialCommunicationInterface& communication_interface_;

    std::span<OperationCodeHandlerInfo> op_code_handlers_;
    size_t                              op_code_handler_registering_index_ = 0;

    uint8_t device_id_;
};

}  // namespace serial_communication_framework

#endif  // PROTOCOLHANDLER_H
