#ifndef MASTER_SLAVE_COMMON_H
#define MASTER_SLAVE_COMMON_H

#include "packets.h"
#include "utils/StaticList.h"

namespace serial_communication_framework {

enum class ResponseCode : uint8_t {
    ok,

    unknown_operation_code,
    invalid_arguments,
    payload_missing_parts,
    timed_out,
    corrupted,  // crc matching failed
};

struct ResponseData {
    ResponseCode                                                   response_code;
    utils::StaticList<uint8_t, ResponsePacket::K_PAYLOAD_MAX_SIZE> response_data;
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
