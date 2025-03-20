#ifndef COMMON_PROTOCOL_COMMANDS_H
#define COMMON_PROTOCOL_COMMANDS_H

#include <cstdint>

namespace protocol {

enum class BasicCommands : uint8_t {
    ping = 0,
};

}

#endif  // COMMON_PROTOCOL_COMMANDS_H