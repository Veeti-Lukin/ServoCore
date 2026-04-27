#ifndef COMMON_PROTOCOL_COMMANDS_H
#define COMMON_PROTOCOL_COMMANDS_H

/** This file just includes all the command files so that it is easier to import the commands to some other place */

// Expose the EmptyResponse and EmptyRequest in to same namespace as the other request/response types
#include "serial_communication_framework/command_interface.h"
namespace protocol::commands {

using serial_communication_framework::commands::EmptyRequest;
using serial_communication_framework::commands::EmptyResponse;

}  // namespace protocol::commands

#include "commands/get_param_metadata_command.h"
#include "commands/get_registered_param_ids_command.h"
#include "commands/ping_command.h"
#include "commands/read_parm_value_command.h"
#include "commands/write_param_value_command.h"

#endif  // COMMON_PROTOCOL_COMMANDS_H