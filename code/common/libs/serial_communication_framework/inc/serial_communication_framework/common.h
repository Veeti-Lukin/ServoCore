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
    malformed_response     = 0x04,  // master could not deserialize slave's response payload
    malformed_request      = 0x05,  // slave could not deserialize master's request payload

    FIRST_INTERNAL         = timed_out,
    LAST_INTERNAL          = malformed_request,
    // ####################################################################

    // Something weird happened on the slave side, not directly in any way responsible to what master requested
    unexpected_local_error = LAST_INTERNAL + 1,

    invalid_id             = LAST_INTERNAL + 2,
    out_of_bounds          = LAST_INTERNAL + 3,
    type_mismatch          = LAST_INTERNAL + 4,
    forbidden              = LAST_INTERNAL + 5,

    unset_default_value    = 0xFF,
};

struct PacketCounters {
    uint64_t received  = 0;
    uint64_t corrupted = 0;
    uint64_t valid     = 0;
};

// A request whose header crc failed carries no trustworthy receiver id, so it can only ever be counted
// in all_requests. Every request that gets past the header check can be attributed, including one whose
// payload turns out to be corrupt.
struct SlaveCommunicationStatistics {
    PacketCounters all_requests;              // every request seen on the bus
    PacketCounters requests_for_this_device;  // the subset this device could tell were its own

    // Requests this device handled whose answer was only ready after the slave side timeout. The answer
    // is dropped instead of sent late, leaving the master to run to its own timeout.
    uint64_t dropped_late_answers = 0;
};

// Every response the master receives answers a request it sent, so there is nothing to attribute.
struct MasterCommunicationStatistics {
    PacketCounters responses;

    // Requests that got no complete response before the master side timeout expired.
    uint64_t timed_out_requests = 0;
};

// TODO if band with estimation is added calculate the timeouts based on how long message should take to send + handling
// time
// Slave's timeout must always be less than masters timeout, otherwise there is a possibility that master timeouts
// and slave answers after the master has timeout which messes up whole communication
constexpr size_t K_MASTER_TIMEOUT_MS = 100;
constexpr size_t K_SLAVE_TIMEOUT_MS  = K_MASTER_TIMEOUT_MS / 2;

}  // namespace serial_communication_framework

#endif  // MASTER_SLAVE_COMMON_H
