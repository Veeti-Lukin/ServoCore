
#ifndef COMMON_LIBS_SERIAL_COMMUNICATION_FRAMEWORK_REQUESTPAYLOADBASE_H
#define COMMON_LIBS_SERIAL_COMMUNICATION_FRAMEWORK_REQUESTPAYLOADBASE_H

#include <cstdint>
#include <span>

#include "serial_communication_framework/common.h"
#include "serial_communication_framework/packets.h"

namespace serial_communication_framework::commands {

enum class ParsingError {
    no_error,

    payload_missing_bytes,
    payload_does_not_fit,
    string_missing_null_termination,
};

//
//
//
//
//

struct RequestBase {
    using ParsingError                                                     = ParsingError;

    virtual ~RequestBase()                                                 = default;

    virtual ParsingError       deserialize(std::span<uint8_t> bytes)       = 0;
    virtual std::span<uint8_t> serialize(std::span<uint8_t> target_buffer) = 0;
};

struct ResponseBase {
    using ParsingError                                                     = ParsingError;

    virtual ~ResponseBase()                                                = default;

    virtual ParsingError       deserialize(std::span<uint8_t> bytes)       = 0;
    virtual std::span<uint8_t> serialize(std::span<uint8_t> target_buffer) = 0;
    explicit                   operator bool() const { return response_code == ResponseCode::ok; }

    ResponseCode response_code = ResponseCode::unset_default_value;
};

//
//
//
//
//

struct EmptyResponse final : ResponseBase {
    std::span<uint8_t> serialize(std::span<uint8_t> target_buffer) override { return target_buffer.subspan(0, 0); }

    ParsingError deserialize(std::span<uint8_t> bytes) override { return ParsingError::no_error; }
};

struct EmptyRequest final : RequestBase {
    ParsingError       deserialize(std::span<uint8_t> bytes) override { return ParsingError::no_error; }
    std::span<uint8_t> serialize(std::span<uint8_t> target_buffer) override { return target_buffer.subspan(0, 0); }
};

//
//
//
//
//

template <typename T>
concept DerivedRequestType = std::derived_from<T, RequestBase>;

template <typename T>
concept DerivedResponseType = std::derived_from<T, ResponseBase>;

template <DerivedRequestType T_Request, DerivedResponseType T_Response, uint8_t OpCode>
struct Command {
    using Request                      = T_Request;
    using Response                     = T_Response;

    static constexpr uint8_t K_OP_CODE = OpCode;

    // THIS STRUCT IS NOT MEANT TO BE INSTANTIATED
    // AND INSTEAD JUST SERVES AS A META TYPE FOR OPCODE, REQUEST AND RESPONSE TYPE
    Command()                          = delete;
};

template <typename T>
concept CommandType =
    requires {
        typename T::Request;
        typename T::Response;
        { T::K_OP_CODE } -> std::convertible_to<uint8_t>;
    } && DerivedRequestType<typename T::Request> && DerivedResponseType<typename T::Response> &&
    (!std::is_abstract_v<typename T::Request>) && (!std::is_abstract_v<typename T::Response>);
;

}  // namespace serial_communication_framework::commands

#endif  // COMMON_LIBS_SERIAL_COMMUNICATION_FRAMEWORK_REQUESTPAYLOADBASE_H
