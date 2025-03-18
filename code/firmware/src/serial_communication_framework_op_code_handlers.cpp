#include "serial_communication_framework_op_code_handlers.h"

namespace serial_communication_framework_op_code_handlers {

ResponseData setParamValue(std::span<std::uint8_t> request_data) { return {}; }

ResponseData getParamValue(std::span<std::uint8_t> request_data) { return {}; }

ResponseData echo(std::span<std::uint8_t> request_data) {
    return {.response_code = serial_communication_framework::ResponseCode::ok, .response_data = request_data};
}

ResponseData ping(std::span<std::uint8_t> request_data) {
    return {.response_code = serial_communication_framework::ResponseCode::ok, .response_data = {}};
}

}  // namespace serial_communication_framework_op_code_handlers
