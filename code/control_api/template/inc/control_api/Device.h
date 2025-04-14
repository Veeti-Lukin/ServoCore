#ifndef CONTROL_API_DEVICE_H
#define CONTROL_API_DEVICE_H

#include <cstdint>

#include "parameter_system/ParameterDeclaration.h"
#include "parameter_system/definitions.h"
#include "parameter_system/parameter_type_mappings.h"
#include "protocol/commands.h"
#include "protocol/requests.h"
#include "serial_communication_framework/MasterHandler.h"
#include "utils/StaticList.h"

namespace servo_core_control_api {

using ParameterMetaData = parameter_system::ParameterMetaData;

using Commands          = protocol::Commands;
using ParameterID       = parameter_system::ParameterID;

template <typename T>
concept ParameterDeclaration = requires(T t) {
    { T::param_type };  // The param_type static member must exist
    {
        t.id
    };  //-> std::same_as<ParameterID>;  // Non-static member id of type ParameterID //TODO Type checking does nto work
};

// TODO RENAME EVERY METHOD THAT SENDS SOMETHING TO DEVICE AND WAITS FOR RESULT TO FETCH OR TRYGET INSTEAD OF GET AND
// SET

// Forward declaration
class Context;

class Device {
    friend Context;

public:
    ~Device();

    void    executeCommand();
    uint8_t getId();

    utils::StaticList<ParameterID, parameter_system::K_MAX_PARAMETER_ID> getRegisteredParameterIds();
    ParameterMetaData                                                    getParameterMetaData(ParameterID id);

    template <ParameterDeclaration T_Declaration>
    auto readParameterValue(T_Declaration parameter_declaration) {
        using serial_communication_framework::ResponseCode;
        using serial_communication_framework::ResponseData;

        using CppType         = typename parameter_system::MapParameterTypeToCppType<T_Declaration::param_type>::type;

        ResponseData response = communication_handler_->sendRequestAndReceiveResponseBlocking(
            device_id_, protocol::requests::ReadParameterValue::op_code,
            protocol::requests::ReadParameterValue::RequestPayload(parameter_declaration.id, T_Declaration::param_type)
                .serialize());

        if (response.response_code != ResponseCode::ok) {
            return CppType();
        }

        protocol::requests::ReadParameterValue::ResponsePayload response_payload(response.response_data);

        CppType value;
        std::memcpy(&value, response_payload.serialized_value, sizeof(CppType));

        return value;
    }

    template <ParameterDeclaration T_Declaration>
    auto writeParameterValue(T_Declaration parameter_declaration) {
        using CppType = typename parameter_system::MapParameterTypeToCppType<T_Declaration::param_type>::type;

        return CppType();
    }

    static constexpr size_t K_MAX_DEVICE_ID = std::numeric_limits<uint8_t>::max();

private:
    explicit Device(uint8_t id, serial_communication_framework::MasterHandler& communication_handler);

    uint8_t                                        device_id_;
    serial_communication_framework::MasterHandler* communication_handler_;
};

}  // namespace servo_core_control_api

#endif  // CONTROL_API_DEVICE_H