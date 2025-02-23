#include "comm_protocol_op_code_handlers.h"

comm_protocol::ResponseData comm_protocol_op_code_handlers::setParamValue(std::span<std::uint8_t> request_data) {
    return comm_protocol::ResponseData();
}
comm_protocol::ResponseData comm_protocol_op_code_handlers::getParamValue(std::span<std::uint8_t> request_data) {
    return comm_protocol::ResponseData();
}
comm_protocol::ResponseData comm_protocol_op_code_handlers::echo(std::span<std::uint8_t> request_data) {
    return {.response_code = comm_protocol::ResponseCode::ok, .response_data = request_data};
}