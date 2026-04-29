#include "protocol/commands/get_registered_param_ids_command.h"

#include "assert/assert.h"

namespace protocol::commands {

// TODO refactor to follow same style as for other commands
serial_communication_framework::commands::ResponseBase::ParsingError GetRegisteredParamIdsResponse::deserialize(
    std::span<uint8_t> bytes) {
    for (const uint8_t& byte : bytes) {
        ids.pushBack(byte);
    }

    return ParsingError::no_error;
}

std::span<uint8_t> GetRegisteredParamIdsResponse::serialize(std::span<uint8_t> target_buffer) {
    ASSERT_WITH_MESSAGE(target_buffer.size() >= ids.size(), "Target buffer is too small");

    for (size_t i = 0; i < ids.size(); i++) {
        target_buffer[i] = ids[i];
    }

    return target_buffer.subspan(0, ids.size());
}

}  // namespace protocol::commands