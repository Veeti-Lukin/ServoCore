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
    out_of_bounds,
    type_mismatch,
    not_allowed,
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

}  // namespace serial_communication_framework

#endif  // MASTER_SLAVE_COMMON_H
