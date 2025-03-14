#ifndef PROTOCOLHANDLER_H
#define PROTOCOLHANDLER_H

#include <cstdint>
#include <span>

#include "drivers/interfaces/SerialBufferedCommunicationInterface.h"
#include "serial_communication_framework/common.h"

namespace serial_communication_framework {

using OperationCodeHandler = ResponseData (*)(std::span<std::uint8_t> request_data);

struct OperationCodeHandlerInfo {
    uint8_t              operation_code;
    OperationCodeHandler handler;
};

class SlaveHandler {
public:
    explicit SlaveHandler(std::span<uint8_t> tx_buffer, std::span<uint8_t> rx_buffer,
                          std::span<OperationCodeHandlerInfo>                        op_code_handler_buffer,
                          drivers::interfaces::SerialBufferedCommunicationInterface& communication_interface);
    ~        SlaveHandler() = default;

    void registerHandler(uint8_t op_code, OperationCodeHandler handler);

    void run();

private:
    OperationCodeHandler getOpcodeHandler(uint8_t op_code);

    std::span<uint8_t> tx_buffer_;
    std::span<uint8_t> rx_buffer_;

    drivers::interfaces::SerialBufferedCommunicationInterface& communication_interface_;

    std::span<OperationCodeHandlerInfo> op_code_handlers_;
    size_t                              op_code_handler_registering_index_ = 0;
};

}  // namespace serial_communication_framework

#endif  // PROTOCOLHANDLER_H
