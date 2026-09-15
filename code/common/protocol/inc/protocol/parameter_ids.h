#ifndef COMMON_PROTOCOL_PARAMETER_IDS_H
#define COMMON_PROTOCOL_PARAMETER_IDS_H

#include "parameter_system/common.h"

namespace protocol {

enum class ParameterIds : parameter_system::ParameterID {
    // ************************ TESTING PARAMETERS ********************************
    // TODO REMOVE
    test_uint8      = 0x02,
    test_uint16     = 0x03,
    test_uint32     = 0x04,
    test_float      = 0x05,
    test_bool       = 0x06,
    loob_back       = 0x07,

    // ************************ USB PARAMETERS ********************************
    usb_configured  = 0x10,
    usb_bus_resets  = 0x11,
    usb_tx_drops    = 0x12,
    usb_rx_overruns = 0x13,

    // ************************ MOTOR PARAMETERS ********************************
};

}

#endif  // COMMON_PROTOCOL_PARAMETER_IDS_H
