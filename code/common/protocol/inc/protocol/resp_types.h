#ifndef RESP_TYPES_H
#define RESP_TYPES_H

#include <cstdint>
#include <span>

namespace protocol {

struct RespBase {
    virtual ~    RespBase()                                  = default;
    virtual void serialize(std::span<uint8_t> target_buffer) = 0;
};

struct SetParamResp {};

struct ReadParamResp {
    uint8_t param_id;
};

}  // namespace protocol

#endif  // RESP_TYPES_H
