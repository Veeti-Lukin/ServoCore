#ifndef COMMON_PROTOCOL_COMMANDS_OP_CODES_H
#define COMMON_PROTOCOL_COMMANDS_OP_CODES_H

#include <cstdint>

namespace protocol::commands::internal {

enum class OperationCodes : uint8_t {
    /** BASIC COMMANDS **/
    ping                               = 0x01,
    reboot                             = 0x02,
    boot_to_pico_usb_mass_storage_mode = 0x03,

    /** PARAMETER ACCESS **/
    write_parameter_value              = 0x20,
    read_parameter_value               = 0x21,
    get_parameter_metadata             = 0x22,
    get_all_registered_parameter_ids   = 0x23,

    /** MOTOR COMMANDS **/
    start_motor                        = 0x40,
    stop_motor                         = 0x41,
    // stop_motor_fading                  = 0x42,
};

}  // namespace protocol::commands::internal

#endif  // COMMON_PROTOCOL_COMMANDS_OP_CODES_H
