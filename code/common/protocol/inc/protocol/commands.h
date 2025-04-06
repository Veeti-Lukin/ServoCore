#ifndef COMMON_PROTOCOL_COMMANDS_H
#define COMMON_PROTOCOL_COMMANDS_H

#include <cstdint>

namespace protocol {

enum class BasicCommands : uint8_t {
    ping = 0,
};

enum class ParameterCommands : uint8_t {
    write,
    read,
    get_metadata,
    get_ids,
};

}  // namespace protocol

#endif  // COMMON_PROTOCOL_COMMANDS_H