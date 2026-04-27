#ifndef MASTER_SLAVE_COMMON_H
#define MASTER_SLAVE_COMMON_H

#include "packets.h"
#include "utils/StaticList.h"

namespace serial_communication_framework {

enum class ResponseCode : uint8_t {
    ok                     = 0x00,

    // ############# COMMAND HANDLERS SHOULD NOT RETURN THESE #############
    // ## THESE ARE MEANT TO BE RETURNED AUTOMATICALLY IF AN ERROR OCCURS #
    timed_out              = 0x01,
    corrupted              = 0x02,  // crc matching failed
    unknown_operation_code = 0x03,

    FIRST_INTERNAL         = timed_out,
    LAST_INTERNAL          = unknown_operation_code,
    // ####################################################################

    // Something weird happened on the slave side, not directly in any way responsible to what master requested
    unexpected_local_error = LAST_INTERNAL + 1,

    invalid_id             = LAST_INTERNAL + 2,
    out_of_bounds          = LAST_INTERNAL + 3,
    type_mismatch          = LAST_INTERNAL + 4,
    forbidden              = LAST_INTERNAL + 5,

    unset_default_value    = 0xFF,
};

struct CommunicationStatistics {
    uint64_t total_packets_received     = 0;
    uint64_t corrupted_packets_received = 0;
    uint64_t valid_packets_received     = 0;
    uint64_t timed_out_packets          = 0;
};

// TODO if band with estimation is added calculate the timeouts based on how long message should take to send + handling
// time
// Slave's timeout must always be less than masters timeout, otherwise there is a possibility that master timeouts
// and slave answers after the master has timeout which messes up whole communication
constexpr size_t K_MASTER_TIMEOUT_MS = 100;
constexpr size_t K_SLAVE_TIMEOUT_MS  = K_MASTER_TIMEOUT_MS / 2;

}  // namespace serial_communication_framework

#endif  // MASTER_SLAVE_COMMON_H
