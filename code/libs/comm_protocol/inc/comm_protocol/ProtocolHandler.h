#ifndef PROTOCOLHANDLER_H
#define PROTOCOLHANDLER_H

#include <cstdint>
#include <span>

#include "comm_protocol/protocol_definition.h"
#include "drivers/interfaces/SerialBufferedCommunicationInterface.h"

namespace comm_protocol {

enum class ResponseCode : uint8_t {
    ok,

    unknown_operation_code,
    invalid_arguments,

    placeholder1,
    placeholder2,
    placeholder3,
    placeholder4,
};

struct ResponseData {
    ResponseCode       response_code;
    std::span<uint8_t> response_data;
};

using OperationCodeHandler = ResponseData (*)(std::span<std::uint8_t> request_data);

struct OperationCodeHandlerInfo {
    uint8_t              operation_code;
    OperationCodeHandler handler;
};

class ProtocolHandler {
public:
    explicit ProtocolHandler(std::span<uint8_t> tx_buffer, std::span<uint8_t> rx_buffer,
                             std::span<OperationCodeHandlerInfo>                        op_code_handler_buffer,
                             drivers::interfaces::SerialBufferedCommunicationInterface& communication_interface);
    ~        ProtocolHandler() = default;

    void registerHandler(uint8_t op_code, OperationCodeHandler handler);

    void update();

private:
    OperationCodeHandler                getOpcodeHandler(uint8_t op_code);
    static protocol_definitions::Packet constructResponsePacket(ResponseData response_data);

    std::span<uint8_t> tx_buffer_;
    std::span<uint8_t> rx_buffer_;

    drivers::interfaces::SerialBufferedCommunicationInterface& communication_interface_;

    std::span<OperationCodeHandlerInfo> op_code_handlers_;
    size_t                              op_code_handler_registering_index_ = 0;
};

}  // namespace comm_protocol

#endif  // PROTOCOLHANDLER_H
