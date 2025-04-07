#ifndef COMMON_PROTOCOL_COMMANDS_H
#define COMMON_PROTOCOL_COMMANDS_H

#include <cstdint>

#include "protocol/operation_codes.h"

namespace protocol {

enum class Commands : uint8_t {
    ping = static_cast<uint8_t>(OperationCodes::ping),
};

}  // namespace protocol

#endif  // COMMON_PROTOCOL_COMMANDS_H