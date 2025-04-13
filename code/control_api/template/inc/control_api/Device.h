#ifndef CONTROL_API_DEVICE_H
#define CONTROL_API_DEVICE_H

#include <cstdint>

#include "parameter_system/ParameterDeclaration.h"
#include "parameter_system/definitions.h"
#include "serial_communication_framework/MasterHandler.h"
#include "utils/StaticList.h"

namespace servo_core_control_api {

using ParameterMetaData = parameter_system::ParameterMetaData;

using ParameterID       = parameter_system::ParameterID;

template <typename T>
concept ParameterDeclaration = requires {
    { T::param_type };  // The param_type member must exist
};

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
        using CppType = typename parameter_system::MapParameterTypeToCppType<T_Declaration::param_type>::type;

        return CppType();
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