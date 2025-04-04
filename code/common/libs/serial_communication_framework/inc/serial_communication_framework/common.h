#ifndef MASTER_SLAVE_COMMON_H
#define MASTER_SLAVE_COMMON_H

namespace serial_communication_framework {

enum class ResponseCode : uint8_t {
    ok,

    unknown_operation_code,
    invalid_arguments,
    timed_out,
    corrupted,  // crc matching failed
};

struct ResponseData {
    ResponseCode       response_code;
    std::span<uint8_t> response_data;
};

struct CommunicationStatistics {
    uint64_t total_packets_received     = 0;
    uint64_t corrupted_packets_received = 0;
    uint64_t valid_packets_received     = 0;
    uint64_t timed_out_packets          = 0;
};

}  // namespace serial_communication_framework

#endif  // MASTER_SLAVE_COMMON_H
