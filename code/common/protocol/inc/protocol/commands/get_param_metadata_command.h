#ifndef COMMON_PROTOCOL_GET_PARAM_METADATA_COMMAND_H
#define COMMON_PROTOCOL_GET_PARAM_METADATA_COMMAND_H

#include "parameter_system/common.h"
#include "protocol/commands/internal/op_codes.h"
#include "serial_communication_framework/command_interface.h"

namespace protocol::commands {

struct GetParamMetadataRequest : serial_communication_framework::commands::RequestBase {
    parameter_system::ParameterID parameter_id;

    ParsingError       deserialize(std::span<uint8_t> bytes) override;
    std::span<uint8_t> serialize(std::span<uint8_t> target_buffer) override;
};

struct GetParamMetadataResponse : serial_communication_framework::commands::ResponseBase {
    parameter_system::ParameterMetaData meta_data;

    ParsingError       deserialize(std::span<uint8_t> bytes) override;
    std::span<uint8_t> serialize(std::span<uint8_t> target_buffer) override;
};

using GetParamMetadata =
    serial_communication_framework::commands::Command<GetParamMetadataRequest, GetParamMetadataResponse,
                                                      static_cast<uint8_t>(
                                                          internal::OperationCodes::get_parameter_metadata)>;

}  // namespace protocol::commands

#endif  // COMMON_PROTOCOL_GET_PARAM_METADATA_COMMAND_H
