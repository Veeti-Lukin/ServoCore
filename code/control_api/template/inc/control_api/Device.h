#ifndef CONTROL_API_DEVICE_H
#define CONTROL_API_DEVICE_H

#include <cstdint>

#include "parameter_system/definitions.h"
#include "serial_communication_framework/MasterHandler.h"
#include "utils/StaticList.h"

namespace servo_core_control_api {

using ParameterMetaData = parameter_system::ParameterMetaData;

// Forward declaration
class Context;

class Device {
    friend Context;

public:
    ~Device();

    void    executeCommand();
    uint8_t getId();

    utils::StaticList<uint8_t, parameter_system::K_MAX_PARAMETER_ID> getRegisteredParameterIds();

    ParameterMetaData getParameterMetaData(uint8_t id);

    static constexpr size_t K_MAX_DEVICE_ID = std::numeric_limits<uint8_t>::max();

private:
    explicit Device(uint8_t id, serial_communication_framework::MasterHandler& communication_handler);

    uint8_t                                        device_id_;
    serial_communication_framework::MasterHandler* communication_handler_;
};

}  // namespace servo_core_control_api

#endif  // CONTROL_API_DEVICE_H