#include "control_api/windows/Context.h"

namespace servo_core_control_api::windows {

Context::Context(std::string serial_port_name)
    // serial communication driver must be initialized first before initializing the base class of the context
    : serial_communication_driver_{serial_port_name.c_str()},
      servo_core_control_api::Context(serial_communication_driver_, program_uptime_clock_) {}

void Context::open() {
    servo_core_control_api::Context::open();
    // TODO Add com port opening here
}

}  // namespace servo_core_control_api::windows