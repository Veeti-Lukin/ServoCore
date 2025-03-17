#ifndef serial_communication_framework_OP_CODE_HANDLERS_H
#define serial_communication_framework_OP_CODE_HANDLERS_H

#include "serial_communication_framework/SlaveHandler.h"

namespace serial_communication_framework_op_code_handlers {

serial_communication_framework::ResponseData setParamValue(std::span<std::uint8_t> request_data);
serial_communication_framework::ResponseData getParamValue(std::span<std::uint8_t> request_data);

serial_communication_framework::ResponseData echo(std::span<std::uint8_t> request_data);

}  // namespace serial_communication_framework_op_code_handlers

#endif  // serial_communication_framework_OP_CODE_HANDLERS_H
