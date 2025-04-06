#ifndef CONTROL_API_CONTEXT_H
#define CONTROL_API_CONTEXT_H

#include <cstdint>
#include <optional>

#include "control_api/Device.h"
#include "drivers/interfaces/BufferedSerialCommunicationInterface.h"
#include "serial_communication_framework/MasterHandler.h"

namespace servo_core_control_api {

class Context {
public:
    explicit Context(drivers::interfaces::BufferedSerialCommunicationInterface& comm_interface);
    ~        Context();

    void open() {}

    [[nodiscard]] std::optional<Device> tryFindDeviceById(uint8_t id);

    /* TODO: should something like this be here? Who allocates the buffer?
    const std::span<Device*> findAllConnectedDevices();*/

private:
    serial_communication_framework::MasterHandler communication_handler;
};

}  // namespace servo_core_control_api

#endif  // CONTROL_API_CONTEXT_H
