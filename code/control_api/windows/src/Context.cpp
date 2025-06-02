#include "control_api/windows/Context.h"

#include <iostream>

#include "debug_print/debug_print.h"

namespace servo_core_control_api::windows {

void debugPrintPutChar(char c) { std::cout << c; }
void debugPrintFlush() { std::cout << std::flush; }

Context::Context(std::string serial_port_name)
    // serial communication driver must be initialized first before initializing the base class of the context
    : serial_communication_driver_{serial_port_name.c_str()},
      servo_core_control_api::Context(serial_communication_driver_, program_uptime_clock_) {
    debug_print::connectPutCharAndFlushFunctions(debugPrintPutChar, debugPrintFlush);
}

void Context::open() {
    servo_core_control_api::Context::open();
    // TODO Add com port opening here
}

}  // namespace servo_core_control_api::windows