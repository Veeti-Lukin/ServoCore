#include "control_api/Context.h"

#include "protocol/commands.h"

namespace servo_core_control_api {

Context::Context(drivers::interfaces::BufferedSerialCommunicationInterface& comm_interface,
                 drivers::interfaces::ClockInterface&                       comm_timeout_clock)
    : communication_handler{comm_interface, comm_timeout_clock} {}

Context::~Context() {}

void Context::open() { communication_handler.init(); }

std::optional<Device> Context::tryFindDeviceById(uint8_t id) {
    using serial_communication_framework::ResponseCode;
    using serial_communication_framework::ResponseData;

    ResponseData response = communication_handler.sendRequestAndReceiveResponseBlocking(
        id, static_cast<uint8_t>(protocol::Commands::ping), {});

    if (response.response_code != ResponseCode::ok) {  // TODO what if the result is "corrupted"?
        // Return empty std::optional
        return {};
    }

    return Device(id, communication_handler);
}

}  // namespace servo_core_control_api