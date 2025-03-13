#ifndef COMM_PROTOCOL_OP_CODE_HANDLERS_H
#define COMM_PROTOCOL_OP_CODE_HANDLERS_H

#include "comm_protocol/SlaveProtocolHandler.h"

namespace comm_protocol_op_code_handlers {
/*
comm_protocol::HandlerResponseData setParamValue(std::span<std::uint8_t> request_data);
comm_protocol::HandlerResponseData getParamValue(std::span<std::uint8_t> request_data);

comm_protocol::HandlerResponseData echo(std::span<std::uint8_t> request_data);*/
struct print_and_return_plus_1_req {
    uint8_t num;
};
struct print_and_return_plus_1_resp {
    uint8_t num;
};
comm_protocol::ResponseCode print_and_return_plus1(print_and_return_plus_1_req   req,
                                                   print_and_return_plus_1_resp& resp_out);

}  // namespace comm_protocol_op_code_handlers

#endif  // COMM_PROTOCOL_OP_CODE_HANDLERS_H
