#include "protocol/commands/write_param_value_command.h"

#include <cstring>

#include "assert/assert.h"

namespace protocol::commands {

serial_communication_framework::commands::RequestBase::ParsingError WriteParamValueRequest::deserialize(
    std::span<uint8_t> bytes) {
    constexpr size_t K_HEADER_SIZE = sizeof(parameter_id) + sizeof(expected_value_type);
    if (bytes.size_bytes() < K_HEADER_SIZE) return ParsingError::payload_missing_bytes;

    size_t idx = 0;
    std::memcpy(&parameter_id, &bytes[idx], sizeof(parameter_id));
    idx += sizeof(parameter_id);

    std::memcpy(&expected_value_type, &bytes[idx], sizeof(expected_value_type));
    idx += sizeof(expected_value_type);

    // Trailing value bytes — variable length, fill the take/put buffer
    const size_t trailing_byte_count = bytes.size_bytes() - idx;
    if (trailing_byte_count > K_RAW_BUFF_SIZE) return ParsingError::payload_does_not_fit;
    
    valid_byte_count = trailing_byte_count;
    std::memcpy(raw_bytes, &bytes[idx], valid_byte_count);

    return ParsingError::no_error;
}

std::span<uint8_t> WriteParamValueRequest::serialize(std::span<uint8_t> target_buffer) {
    ASSERT_WITH_MESSAGE(
        sizeof(parameter_id) + sizeof(expected_value_type) + valid_byte_count <= target_buffer.size_bytes(),
        "Target buffer is too small");

    size_t idx = 0;
    std::memcpy(&target_buffer[idx], &parameter_id, sizeof(parameter_id));
    idx += sizeof(parameter_id);

    std::memcpy(&target_buffer[idx], &expected_value_type, sizeof(expected_value_type));
    idx += sizeof(expected_value_type);

    std::memcpy(&target_buffer[idx], raw_bytes, valid_byte_count);
    idx += valid_byte_count;

    return target_buffer.subspan(0, idx);
}

}  // namespace protocol::commands
