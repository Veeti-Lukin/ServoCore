#include "protocol/commands/get_param_metadata_command.h"

#include <cstring>

#include "assert/assert.h"
#include "protocol/parameters.h"

namespace protocol::commands {

serial_communication_framework::commands::RequestBase::ParsingError GetParamMetadataRequest::deserialize(
    std::span<uint8_t> bytes) {
    if (bytes.size_bytes() < sizeof(parameter_id)) return ParsingError::payload_missing_bytes;

    size_t idx = 0;
    std::memcpy(&parameter_id, &bytes[idx], sizeof(parameter_id));

    return ParsingError::no_error;
}

std::span<uint8_t> GetParamMetadataRequest::serialize(std::span<uint8_t> target_buffer) {
    ASSERT_WITH_MESSAGE(sizeof(parameter_id) <= target_buffer.size_bytes(), "Target buffer is too small");

    size_t idx = 0;
    std::memcpy(&target_buffer[idx], &parameter_id, sizeof(parameter_id));
    idx += sizeof(parameter_id);

    return target_buffer.subspan(0, idx);
}

serial_communication_framework::commands::ResponseBase::ParsingError GetParamMetadataResponse::deserialize(
    std::span<uint8_t> bytes) {
    // plus 1 because the name must have at least null termination
    constexpr size_t K_MINIMUM_BUFFER_SIZE = sizeof(parameter_system::ParameterMetaData::id) +
                                             sizeof(parameter_system::ParameterMetaData::type) +
                                             sizeof(parameter_system::ParameterMetaData::value_type) +
                                             sizeof(parameter_system::ParameterMetaData::read_write_access) + 1;
    if (bytes.size_bytes() < K_MINIMUM_BUFFER_SIZE) return ParsingError::payload_missing_bytes;

    size_t idx = 0;
    std::memcpy(&meta_data.id, &bytes[idx], sizeof(meta_data.id));
    idx += sizeof(meta_data.id);

    std::memcpy(&meta_data.type, &bytes[idx], sizeof(meta_data.type));
    idx += sizeof(meta_data.type);

    std::memcpy(&meta_data.value_type, &bytes[idx], sizeof(meta_data.value_type));
    idx += sizeof(meta_data.value_type);

    std::memcpy(&meta_data.read_write_access, &bytes[idx], sizeof(meta_data.read_write_access));
    idx += sizeof(meta_data.read_write_access);

    // Name string follows — variable length, null-terminated
    for (size_t name_idx = 0; idx < bytes.size_bytes(); idx++, name_idx++) {
        char c                   = static_cast<char>(bytes[idx]);
        meta_data.name[name_idx] = c;
        if (c == '\0') {
            return ParsingError::no_error;
        }
    }
    return ParsingError::string_missing_null_termination;
}

std::span<uint8_t> GetParamMetadataResponse::serialize(std::span<uint8_t> target_buffer) {
    constexpr size_t K_REQUIRED_BUFFER_SIZE = sizeof(parameter_system::ParameterMetaData::id) +
                                              sizeof(parameter_system::ParameterMetaData::type) +
                                              sizeof(parameter_system::ParameterMetaData::value_type) +
                                              sizeof(parameter_system::ParameterMetaData::read_write_access) +
                                              parameter_system::ParameterMetaData::K_PARAMETER_NAME_MAX_LENGTH;
    ASSERT_WITH_MESSAGE(target_buffer.size_bytes() >= K_REQUIRED_BUFFER_SIZE, "Target buffer is too small");

    size_t idx = 0;
    std::memcpy(&target_buffer[idx], &meta_data.id, sizeof(meta_data.id));
    idx += sizeof(meta_data.id);

    std::memcpy(&target_buffer[idx], &meta_data.type, sizeof(meta_data.type));
    idx += sizeof(meta_data.type);

    std::memcpy(&target_buffer[idx], &meta_data.value_type, sizeof(meta_data.value_type));
    idx += sizeof(meta_data.value_type);

    std::memcpy(&target_buffer[idx], &meta_data.read_write_access, sizeof(meta_data.read_write_access));
    idx += sizeof(meta_data.read_write_access);

    // Name string, copy until and including the null terminator
    for (const char& c : meta_data.name) {
        target_buffer[idx] = c;
        idx++;
        if (c == '\0') break;
    }

    return target_buffer.subspan(0, idx);
}

}  // namespace protocol::commands
