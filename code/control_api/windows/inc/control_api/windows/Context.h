#ifndef CONTROL_API_WINDOWS_CONTEXT_H
#define CONTROL_API_WINDOWS_CONTEXT_H

#include <string>

#include "control_api/Context.h"
#include "control_api/windows/internal/BufferedAsyncSerialportDriver.h"

namespace servo_core_control_api::windows {

class Context : public servo_core_control_api::Context {
public:
    explicit Context(std::string serial_port_name);

private:
    internal::BufferedAsyncSerialPortDriver serial_communication_driver_;
};

}  // namespace servo_core_control_api::windows

#endif  // CONTEXT_H
