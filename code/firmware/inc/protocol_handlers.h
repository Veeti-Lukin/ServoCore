#ifndef serial_communication_framework_OP_CODE_HANDLERS_H
#define serial_communication_framework_OP_CODE_HANDLERS_H

#include "serial_communication_framework/SlaveHandler.h"

namespace protocol_handlers {

using serial_communication_framework::ResponseData;

ResponseData ping(std::span<std::uint8_t> request_data);

ResponseData getParamIds(std::span<std::uint8_t> request_data);
ResponseData getParamMetaData(std::span<std::uint8_t> request_data);
ResponseData setParamValue(std::span<std::uint8_t> request_data);
ResponseData getParamValue(std::span<std::uint8_t> request_data);

}  // namespace protocol_handlers

#endif  // serial_communication_framework_OP_CODE_HANDLERS_H
