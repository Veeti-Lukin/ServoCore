#include <cstring>

#include "assert/assert.h"
#include "protocol/commands/read_parm_value_command.h"

namespace protocol::commands {

serial_communication_framework::commands::RequestBase::ParsingError ReadParamValueRequest::deserialize(
    std::span<uint8_t> bytes) {
    if (bytes.size_bytes() < sizeof(parameter_id) + sizeof(expected_value_type)) {
        return ParsingError::payload_missing_bytes;
    }

    size_t idx = 0;
    std::memcpy(&parameter_id, &bytes[idx], sizeof(parameter_id));
    idx += sizeof(parameter_id);

    std::memcpy(&expected_value_type, &bytes[idx], sizeof(expected_value_type));

    return ParsingError::no_error;
}

std::span<uint8_t> ReadParamValueRequest::serialize(std::span<uint8_t> target_buffer) {
    ASSERT_WITH_MESSAGE(sizeof(parameter_id) + sizeof(expected_value_type) <= target_buffer.size_bytes(),
                        "Target buffer is too small");

    size_t idx = 0;
    std::memcpy(&target_buffer[idx], &parameter_id, sizeof(parameter_id));
    idx += sizeof(parameter_id);

    std::memcpy(&target_buffer[idx], &expected_value_type, sizeof(expected_value_type));
    idx += sizeof(expected_value_type);
    return target_buffer.subspan(0, idx);
}

serial_communication_framework::commands::ResponseBase::ParsingError ReadParamValueResponse::deserialize(
    std::span<uint8_t> bytes) {
    ASSERT_WITH_MESSAGE(bytes.size_bytes() <= K_RAW_BUFF_SIZE,
                        "Incoming param value is larger than response raw buffer can hold");

    valid_byte_count = bytes.size_bytes();
    std::memcpy(raw_bytes, bytes.data(), valid_byte_count);

    return ParsingError::no_error;
}

std::span<uint8_t> ReadParamValueResponse::serialize(std::span<uint8_t> target_buffer) {
    ASSERT_WITH_MESSAGE(valid_byte_count <= target_buffer.size_bytes(), "Target buffer is too small");

    std::memcpy(target_buffer.data(), raw_bytes, valid_byte_count);
    return target_buffer.subspan(0, valid_byte_count);
}

}  // namespace protocol::commands
