#ifndef COMMON_PROTOCOL_OPERATION_CODES_H
#define COMMON_PROTOCOL_OPERATION_CODES_H

#include <cstdint>

namespace protocol {

enum class OperationCodes : uint8_t {
    /** BASIC COMMANDS **/
    ping = 0,
    reboot,
    boot_to_pico_usb_mass_storage_mode,

    /** PARAMETER ACCESS **/
    write_parameter_value,
    read_parameter_value,
    get_parameter_metadata,
    get_all_registered_parameter_ids,

    /** MOTOR COMMANDS **/
    start_motor,
    stop_motor,
};

}

#endif  // COMMON_PROTOCOL_OPERATION_CODES_H
