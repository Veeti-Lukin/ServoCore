#ifndef COMMON_PROTOCOL_COMMANDS_REBOOT_H
#define COMMON_PROTOCOL_COMMANDS_REBOOT_H

#include "protocol/commands/internal/op_codes.h"
#include "serial_communication_framework/command_interface.h"

namespace protocol::commands {

using Reboot = serial_communication_framework::commands::Command<serial_communication_framework::commands::EmptyRequest,
                                                                 serial_communication_framework::commands::EmptyResponse,
                                                                 static_cast<uint8_t>(internal::OperationCodes::reboot)>;

}  // namespace protocol::commands

#endif  //  COMMON_PROTOCOL_COMMANDS_REBOOT_H
