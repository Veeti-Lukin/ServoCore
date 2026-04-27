#ifndef COMMON_PROTOCOL_WRITE_PARAM_VALUE_COMMAND_H
#define COMMON_PROTOCOL_WRITE_PARAM_VALUE_COMMAND_H

#include <cstring>

#include "assert/assert.h"
#include "parameter_system/common.h"
#include "protocol/commands/internal/op_codes.h"
#include "serial_communication_framework/command_interface.h"
#include "serial_communication_framework/packets.h"

namespace protocol::commands {

// TODO Deduplicate: this same take/put + raw_bytes + valid_byte_count machinery is
//      duplicated in WriteParamValueRequest (see write_param_value_command.h) and
//      ReadParamValueResponse (see read_parm_value_command.h).
struct WriteParamValueRequest : serial_communication_framework::commands::RequestBase {
private:
    // TODO kinda allocating too much unneeded memory, but at least does not cause problems in the future
    static constexpr size_t K_RAW_BUFF_SIZE = serial_communication_framework::RequestPacket::K_PAYLOAD_MAX_SIZE;

public:
    parameter_system::ParameterID parameter_id;
    // slave returns ResponseCode::type_mismatch if it doesn't
    // match the registered parameter, preventing the slave from misinterpreting bytes. This could happen IE. if
    // control api compilation with protocol is stale compared to firmware
    parameter_system::ParameterValueType expected_value_type;
    uint8_t                              raw_bytes[K_RAW_BUFF_SIZE] = {};
    size_t                               valid_byte_count           = 0;  // Not actually, used for (de)serialization

    ParsingError       deserialize(std::span<uint8_t> bytes) override;
    std::span<uint8_t> serialize(std::span<uint8_t> target_buffer) override;
    // TODO kinda allocating too much unneeded memory, but at least does not cause problems in the future

    /**
     * @brief Store a typed scalar of type @p T as the value to write (Helper for master).
     */
    template <typename T>
    void put(const T& var) {
        static_assert(sizeof(T) <= K_RAW_BUFF_SIZE, "Type does not fit in response buffer");
        std::memcpy(raw_bytes, &var, sizeof(T));
        valid_byte_count = sizeof(T);
    }
};

using WriteParamValue = serial_communication_framework::commands::Command<
    WriteParamValueRequest, serial_communication_framework::commands::EmptyResponse,
    static_cast<uint8_t>(internal::OperationCodes::write_parameter_value)>;

}  // namespace protocol::commands

#endif  // COMMON_PROTOCOL_WRITE_PARAM_VALUE_COMMAND_H
