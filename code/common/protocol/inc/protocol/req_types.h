#ifndef COMMON_PROTOCOL_REQ_TYPES_H
#define COMMON_PROTOCOL_REQ_TYPES_H

#include <cstdint>
#include <span>

namespace protocol {

struct ReqBase {
    virtual ~    ReqBase()                                   = default;
    virtual void serialize(std::span<uint8_t> target_buffer) = 0;
};

struct SetParamReq {
    uint8_t            param_id;
    std::span<uint8_t> value;
    uint8_t            param_type;
};

struct ReadParamReq {
    uint8_t param_id;
};

}  // namespace protocol

#endif  // COMMON_PROTOCOL_REQ_TYPES_H
