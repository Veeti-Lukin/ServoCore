#ifndef COMM_PROTOCOL_OP_CODE_HANDLERS_H
#define COMM_PROTOCOL_OP_CODE_HANDLERS_H

#include "comm_protocol/ProtocolHandler.h"

namespace comm_protocol_op_code_handlers {

comm_protocol::ResponseData setParamValue(std::span<std::uint8_t> request_data);
comm_protocol::ResponseData getParamValue(std::span<std::uint8_t> request_data);

comm_protocol::ResponseData echo(std::span<std::uint8_t> request_data);

}  // namespace comm_protocol_op_code_handlers

#endif  // COMM_PROTOCOL_OP_CODE_HANDLERS_H
