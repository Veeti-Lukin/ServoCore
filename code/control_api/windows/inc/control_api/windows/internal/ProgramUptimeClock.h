#ifndef CONTROL_API_WINDOWS_PROGRAMUPTIMECLOCK_H
#define CONTROL_API_WINDOWS_PROGRAMUPTIMECLOCK_H

#include <chrono>

#include "drivers/interfaces/ClockInterface.h"

namespace servo_core_control_api::windows::internal {

/**
 * @brief Driver class for accessing system monotonic time.
 *
 * This driver class provides a consistent way to query program uptime.
 * It is intended to represent a **monotonic** time source that starts at
 * zero on program startup and always moves forward (never goes backward or wraps).
 *
 * Time returned is typically used for scheduling, timeouts, profiling, etc.
 * Not suitable for wall-clock timestamps.
 */
class ProgramUptimeClock final : public drivers::interfaces::ClockInterface {
public:
     ProgramUptimeClock()          = default;
    ~ProgramUptimeClock() override = default;

    /**
     * @brief Get the program uptime in microseconds.
     *
     * @return Time in microseconds since program start.
     */
    uint64_t uptimeMicroseconds() override;
    /**
     * @brief Get the program uptime in milliseconds.
     *
     * @return Time in milliseconds since program start.
     */
    uint64_t uptimeMilliseconds() override;
    /**
     * @brief Get the program uptime in seconds.
     *
     * @return Time in seconds since program start.
     */
    uint64_t uptimeSeconds() override;

private:
    static std::chrono::time_point<std::chrono::high_resolution_clock> start_time_point_;
};

}  // namespace servo_core_control_api::windows::internal

#endif  // CONTROL_API_WINDOWS_PROGRAMUPTIMECLOCK_H
