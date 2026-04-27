#ifndef CONTROL_API_DEVICE_H
#define CONTROL_API_DEVICE_H

#include <cstdint>

#include "parameter_system/ParameterDeclaration.h"
#include "parameter_system/common.h"
#include "parameter_system/parameter_type_mappings.h"
#include "protocol/commands.h"
#include "serial_communication_framework/MasterHandler.h"
#include "utils/StaticList.h"

namespace servo_core_control_api {

using ParameterMetaData = parameter_system::ParameterMetaData;

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

    uint8_t getId();

    utils::StaticList<ParameterID, parameter_system::K_MAX_PARAMETER_ID> fetchRegisteredParamIds();

    ParameterMetaData fetchParameterMetaData(ParameterID id);

    template <parameter_system::ParameterValueType T_ValueType>
    auto readParameterValue(const parameter_system::ParameterDeclaration<T_ValueType>& declaration) {
        using CppType = parameter_system::MapParameterValueTypeToCppType<T_ValueType>::type;
        using namespace serial_communication_framework;

        protocol::commands::ReadParamValueRequest request;
        request.parameter_id        = declaration.id;
        request.expected_value_type = T_ValueType;

        protocol::commands::ReadParamValueResponse response =
            communication_handler_->sendCommandAndReceiveResponseBlocking<protocol::commands::ReadParamValue>(
                device_id_, request);

        if (response.response_code != ResponseCode::ok) {
            return CppType{};
        }
        return response.take<CppType>();
    }

    template <parameter_system::ParameterValueType T_ValueType>
    serial_communication_framework::ResponseCode writeParameterValue(
        const parameter_system::ParameterDeclaration<T_ValueType>&                   declaration,
        typename parameter_system::MapParameterValueTypeToCppType<T_ValueType>::type value) {
        using CppType = parameter_system::MapParameterValueTypeToCppType<T_ValueType>::type;
        using namespace serial_communication_framework;

        protocol::commands::WriteParamValueRequest request;
        request.parameter_id        = declaration.id;
        request.expected_value_type = T_ValueType;
        request.put<CppType>(value);

        protocol::commands::EmptyResponse response =
            communication_handler_->sendCommandAndReceiveResponseBlocking<protocol::commands::WriteParamValue>(
                device_id_, request);

        return response.response_code;
    }

    static constexpr size_t K_MAX_DEVICE_ID = std::numeric_limits<uint8_t>::max();

private:
    explicit Device(uint8_t id, serial_communication_framework::MasterHandler& communication_handler);

    uint8_t                                        device_id_;
    serial_communication_framework::MasterHandler* communication_handler_;

    /*
    template <typename T_Command>
    typename T_Command::Response executeBlockingCommand(typename T_Command::Request) {
        // TODO create
        // VOis ottaa jonmkun "kartan" responseista mitä master puoli olettaa tulevan laittaista ja error messaget
    niistä
        // Sit jos tuleekin joku muu niin throwais exceptionin että unexpected
    }*/
};

}  // namespace servo_core_control_api

#endif  // CONTROL_API_DEVICE_H