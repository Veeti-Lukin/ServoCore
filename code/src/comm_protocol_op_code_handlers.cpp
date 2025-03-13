#include "comm_protocol_op_code_handlers.h"

#include "debug_print/debug_print.h"

/*
comm_protocol::HandlerResponseData comm_protocol_op_code_handlers::setParamValue(std::span<std::uint8_t> request_data) {
    return comm_protocol::HandlerResponseData();
}
comm_protocol::HandlerResponseData comm_protocol_op_code_handlers::getParamValue(std::span<std::uint8_t> request_data) {
    return comm_protocol::HandlerResponseData();
}
comm_protocol::HandlerResponseData comm_protocol_op_code_handlers::echo(std::span<std::uint8_t> request_data) {
    return {.response_code = comm_protocol::ResponseCode::ok, .response_data = request_data};
}*/

comm_protocol::ResponseCode print_and_return_plus1(
    comm_protocol_op_code_handlers::print_and_return_plus_1_req   req,
    comm_protocol_op_code_handlers::print_and_return_plus_1_resp& resp_out) {
    DEBUG_PRINT("Got: % \n", req.num);
    DEBUG_PRINT("Giving: % \n", req.num + 1);
    resp_out.num = req.num + 1;
    return comm_protocol::ResponseCode::ok;
}