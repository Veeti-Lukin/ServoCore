#ifndef CONTROL_API_WINDOWS_CONTEXT_H
#define CONTROL_API_WINDOWS_CONTEXT_H

#include <string>

#include "control_api/Context.h"
#include "control_api/windows/internal/BufferedAsyncSerialportDriver.h"
#include "control_api/windows/internal/ProgramUptimeClock.h"

namespace servo_core_control_api::windows {

class Context : public servo_core_control_api::Context {
public:
    explicit Context(std::string serial_port_name);
    ~        Context() = default;

    void open() override;

private:
    internal::BufferedAsyncSerialPortDriver serial_communication_driver_;
    internal::ProgramUptimeClock            program_uptime_clock_;
};

}  // namespace servo_core_control_api::windows

#endif  // CONTEXT_H
