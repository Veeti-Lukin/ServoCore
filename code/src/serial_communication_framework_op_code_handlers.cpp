#include "serial_communication_framework_op_code_handlers.h"

serial_communication_framework::ResponseData serial_communication_framework_op_code_handlers::setParamValue(
    std::span<std::uint8_t> request_data) {
    return serial_communication_framework::ResponseData();
}
serial_communication_framework::ResponseData serial_communication_framework_op_code_handlers::getParamValue(
    std::span<std::uint8_t> request_data) {
    return serial_communication_framework::ResponseData();
}
serial_communication_framework::ResponseData serial_communication_framework_op_code_handlers::echo(
    std::span<std::uint8_t> request_data) {
    return {.response_code = serial_communication_framework::ResponseCode::ok, .response_data = request_data};
}