#ifndef PROTOCOLHANDLER_H
#define PROTOCOLHANDLER_H

#include <cstdint>
#include <cstring>
#include <span>

#include "assert/assert.h"
#include "comm_protocol/packets.h"
#include "drivers/interfaces/SerialBufferedCommunicationInterface.h"

namespace comm_protocol {

enum class ResponseCode : uint8_t {
    ok,

    unknown_operation_code,
    invalid_arguments,
};

using OperationCodeHandler        = ResponseCode        (*)(std::span<std::uint8_t> request_data);
using OperationCodeHandlerWrapper = ResponseCode (*)(std::span<uint8_t> raw_request_payload,
                                                     std::span<uint8_t> tx_buffer, void* handler_ptr,
                                                     std::span<uint8_t>& response_payload_out);

struct OperationCodeHandlerInfo {
    uint8_t                     operation_code;
    void*                       handler;
    OperationCodeHandlerWrapper wrapper = nullptr;
};

class SlaveProtocolHandler {
public:
    explicit SlaveProtocolHandler(std::span<uint8_t> tx_buffer, std::span<uint8_t> rx_buffer,
                                  std::span<OperationCodeHandlerInfo>                        op_code_handler_buffer,
                                  drivers::interfaces::SerialBufferedCommunicationInterface& communication_interface);
    ~        SlaveProtocolHandler() = default;

    // void registerHandler(uint8_t op_code, OperationCodeHandler handler);

    template <typename ReqPayload_T, typename RespPayload_T,
              typename OpCodeHandler_T = ResponseCode (*)(ReqPayload_T, RespPayload_T)>
    void registerHandler(uint8_t op_code, OpCodeHandler_T handler);

    void run();

private:
    // OperationCodeHandler getOpcodeHandler(uint8_t op_code);

    std::span<uint8_t> tx_buffer_;
    std::span<uint8_t> rx_buffer_;

    drivers::interfaces::SerialBufferedCommunicationInterface& communication_interface_;

    std::span<OperationCodeHandlerInfo> op_code_handlers_;
    size_t                              op_code_handler_registering_index_ = 0;
};

template <typename ReqPayload_T, typename RespPayload_T, typename OpCodeHandler_T>
void SlaveProtocolHandler::registerHandler(uint8_t op_code, OpCodeHandler_T handler) {
    ASSERT_WITH_MESSAGE(op_code_handler_registering_index_ < op_code_handlers_.size(),
                        "Op code handler registering index out of bounds. Too small buffer");

    // Wrap the generic function to be callable in a uniform way
    auto wrapper = [](std::span<uint8_t> raw_request_payload, std::span<uint8_t> tx_buffer, void* handler_ptr,
                      std::span<uint8_t>& response_payload_out) -> ResponseCode {
        auto casted_handler_ptr = reinterpret_cast<OpCodeHandler_T>(handler_ptr);

        ReqPayload_T request_data;
        memcpy(&request_data, raw_request_payload.data(), sizeof(ReqPayload_T));
        RespPayload_T response_data;

        ResponseCode response_code = casted_handler_ptr(request_data, response_data);

        std::memcpy(tx_buffer.data() + ResponsePacket::K_PAYLOAD_START_OFFSET, &response_data, sizeof(response_data));

        response_payload_out = tx_buffer.subspan(ResponsePacket::K_PAYLOAD_START_OFFSET, sizeof(response_data));

        return response_code;
    };

    op_code_handlers_[op_code_handler_registering_index_] = {
        .operation_code = op_code, .handler = reinterpret_cast<void*>(handler), .wrapper = wrapper};
    op_code_handler_registering_index_++;
}
}  // namespace comm_protocol

#endif  // PROTOCOLHANDLER_H
