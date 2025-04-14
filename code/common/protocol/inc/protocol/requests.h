#ifndef COMMON_PROTOCOL_REQ_TYPES_H
#define COMMON_PROTOCOL_REQ_TYPES_H

#include <cstdint>
#include <cstring>
#include <span>

#include "parameter_system/definitions.h"
#include "parameter_system/parameter_type_mappings.h"
#include "protocol/operation_codes.h"
#include "serial_communication_framework/packets.h"
#include "utils/StaticList.h"

namespace protocol::requests {

constexpr auto K_PAYLOAD_MAX_SIZE = serial_communication_framework::RequestPacket::K_PAYLOAD_MAX_SIZE;

// Base class for payload serialization and deserialization
template <typename DerivedPayload>
struct PayloadBase {
    virtual ~                                              PayloadBase() = default;
    virtual utils::StaticList<uint8_t, K_PAYLOAD_MAX_SIZE> serialize()   = 0;
    static DerivedPayload deserialize(std::span<uint8_t> buffer) { return DerivedPayload{buffer}; }
};

// Empty payload structure
struct EmptyPayload final : PayloadBase<EmptyPayload> {
                                                   EmptyPayload() = default;
    explicit                                       EmptyPayload(std::span<uint8_t>) {}
    utils::StaticList<uint8_t, K_PAYLOAD_MAX_SIZE> serialize() override { return {}; }
};

//
//
//
//
//
//
//
//

// Request to get all registered parameter IDs
struct GetRegisteredParameterIds {
    static constexpr uint8_t op_code = static_cast<uint8_t>(OperationCodes::get_all_registered_parameter_ids);
    using RequestPayload             = EmptyPayload;

    struct ResponsePayload : PayloadBase<ResponsePayload> {
        utils::StaticList<parameter_system::ParameterID, parameter_system::K_MAX_PARAMETER_ID> registered_parameter_ids;

                                                       ResponsePayload() = default;
        explicit                                       ResponsePayload(std::span<uint8_t> buff);
        utils::StaticList<uint8_t, K_PAYLOAD_MAX_SIZE> serialize() override;
    };
};

// Request to get metadata for a specific parameter
struct GetParameterMetaData {
    static constexpr uint8_t op_code = static_cast<uint8_t>(OperationCodes::get_parameter_metadata);

    struct RequestPayload : PayloadBase<RequestPayload> {
        parameter_system::ParameterID parameter_id                      = {};

                                                       RequestPayload() = default;
        explicit                                       RequestPayload(uint8_t id) : parameter_id(id) {}
        explicit                                       RequestPayload(std::span<uint8_t> buffer);
        utils::StaticList<uint8_t, K_PAYLOAD_MAX_SIZE> serialize() override;
    };

    struct ResponsePayload : PayloadBase<ResponsePayload> {
        parameter_system::ParameterMetaData meta_data = {};

                 ResponsePayload()                    = default;
        explicit ResponsePayload(const parameter_system::ParameterMetaData& meta_data) : meta_data(meta_data) {}
        explicit ResponsePayload(std::span<uint8_t> buffer);
        utils::StaticList<uint8_t, K_PAYLOAD_MAX_SIZE> serialize() override;
    };
};

// Request to get metadata for a specific parameter
struct ReadParameterValue {
    static constexpr uint8_t op_code = static_cast<uint8_t>(OperationCodes::read_parameter_value);

    struct RequestPayload : PayloadBase<RequestPayload> {
        parameter_system::ParameterID   id = {};
        parameter_system::ParameterType target_type;

                 RequestPayload() = default;
        explicit RequestPayload(std::span<uint8_t> buffer);
        explicit RequestPayload(parameter_system::ParameterID id, parameter_system::ParameterType type)
            : id(id), target_type(type) {}
        utils::StaticList<uint8_t, K_PAYLOAD_MAX_SIZE> serialize() override;
    };

    struct ResponsePayload : PayloadBase<ResponsePayload> {
        // TODO what the heck
        uint8_t serialized_value[K_PAYLOAD_MAX_SIZE] = {0};

                 ResponsePayload()                   = default;
        explicit ResponsePayload(std::span<uint8_t> buffer);

        utils::StaticList<uint8_t, K_PAYLOAD_MAX_SIZE> serialize() override;
    };
};

}  // namespace protocol::requests

#endif  // COMMON_PROTOCOL_REQ_TYPES_H
