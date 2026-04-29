#ifndef COMMON_PROTOCOL_GET_REGISTERED_PARAMETER_IDS_COMMAND_H
#define COMMON_PROTOCOL_GET_REGISTERED_PARAMETER_IDS_COMMAND_H

#include "parameter_system/common.h"
#include "protocol/commands/internal/op_codes.h"
#include "serial_communication_framework/command_interface.h"

namespace protocol::commands {

struct GetRegisteredParamIdsResponse : serial_communication_framework::commands::ResponseBase {
    utils::StaticList<parameter_system::ParameterID, parameter_system::K_MAX_PARAMETER_ID> ids;

    ParsingError       deserialize(std::span<uint8_t> bytes) override;
    std::span<uint8_t> serialize(std::span<uint8_t> target_buffer) override;
};

using GetRegisteredParamIds = serial_communication_framework::commands::Command<
    serial_communication_framework::commands::EmptyRequest, GetRegisteredParamIdsResponse,
    static_cast<uint8_t>(internal::OperationCodes::get_all_registered_parameter_ids)>;

}  // namespace protocol::commands

#endif  // COMMON_PROTOCOL_GET_REGISTERED_PARAMETER_IDS_COMMAND_H
