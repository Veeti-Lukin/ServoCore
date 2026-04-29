#ifndef COMMON_PROTOCOL_READ_PARM_VALUE_COMMAND_H
#define COMMON_PROTOCOL_READ_PARM_VALUE_COMMAND_H

#include <cstring>

#include "assert/assert.h"
#include "parameter_system/common.h"
#include "protocol/commands/internal/op_codes.h"
#include "serial_communication_framework/command_interface.h"
#include "serial_communication_framework/packets.h"

namespace protocol::commands {

struct ReadParamValueRequest : serial_communication_framework::commands::RequestBase {
    parameter_system::ParameterID parameter_id;
    // slave returns ResponseCode::type_mismatch if it doesn't
    // match the registered parameter, preventing the master from misinterpreting bytes. This could happen IE. if
    // control api compilation with protocol is stale compared to firmware
    parameter_system::ParameterValueType expected_value_type;

    ParsingError       deserialize(std::span<uint8_t> bytes) override;
    std::span<uint8_t> serialize(std::span<uint8_t> target_buffer) override;
};

// TODO Deduplicate: this same take/put + raw_bytes + valid_byte_count machinery is
//      duplicated in WriteParamValueRequest (see write_param_value_command.h) and
//      ReadParamValueResponse (see read_parm_value_command.h).
struct ReadParamValueResponse : serial_communication_framework::commands::ResponseBase {
    // TODO kinda allocating too much unneeded memory, but at least does not cause problems in the future
    static constexpr size_t K_RAW_BUFF_SIZE = serial_communication_framework::RequestPacket::K_PAYLOAD_MAX_SIZE;
    uint8_t                 raw_bytes[K_RAW_BUFF_SIZE] = {};
    size_t                  valid_byte_count           = 0;  // Not actually transmitted, used for (de)serialization

    ParsingError       deserialize(std::span<uint8_t> bytes) override;
    std::span<uint8_t> serialize(std::span<uint8_t> target_buffer) override;

    /**
     * @brief Extract the response value as a typed scalar of type @p T. (Helper for master)
     * @note Asserts that `sizeof(T)` matches the number of bytes the slave sent.
     */
    template <typename T>
    T take() {
        static_assert(K_RAW_BUFF_SIZE >= sizeof(T));
        ASSERT_WITH_MESSAGE(sizeof(T) == valid_byte_count, "Type size mismatch on read");

        T var;
        std::memcpy(&var, raw_bytes, sizeof(T));
        return var;
    }
};

using ReadParamValue =
    serial_communication_framework::commands::Command<ReadParamValueRequest, ReadParamValueResponse,
                                                      static_cast<uint8_t>(
                                                          internal::OperationCodes::read_parameter_value)>;

}  // namespace protocol::commands

#endif  // COMMON_PROTOCOL_READ_PARM_VALUE_COMMAND_H
