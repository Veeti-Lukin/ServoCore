#ifndef FIRMWARE_DRIVERS_PICO_SYSCLOCKDRIVER_H
#define FIRMWARE_DRIVERS_PICO_SYSCLOCKDRIVER_H

#include "drivers/interfaces/ClockInterface.h"

namespace drivers {

/**
 * @brief Driver class for accessing system monotonic time.
 *
 * This driver class provides a consistent way to query system uptime.
 * It is intended to represent a **monotonic** time source that starts at
 * zero on boot and always moves forward (never goes backward or wraps).
 *
 * Time returned is typically used for scheduling, timeouts, profiling, etc.
 * Not suitable for wall-clock timestamps.
 */
class SysClockDriver final : public interfaces::ClockInterface {
public:
     SysClockDriver()          = default;
    ~SysClockDriver() override = default;

    /**
     * @brief Get the system uptime in microseconds.
     *
     * @return Time in microseconds since boot.
     */
    uint64_t uptimeMicroseconds() override;
    /**
     * @brief Get the system uptime in milliseconds.
     *
     * @return Time in milliseconds since boot.
     */
    uint64_t uptimeMilliseconds() override;
    /**
     * @brief Get the system uptime in seconds.
     *
     * @return Time in seconds since boot.
     */
    uint64_t uptimeSeconds() override;
};

}  // namespace drivers

#endif  // FIRMWARE_DRIVERS_PICO_SYSCLOCKDRIVER_H
