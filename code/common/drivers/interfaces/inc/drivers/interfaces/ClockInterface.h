#ifndef COMMON_DRIVERS_INTERFACES_CLOCKINTERFACE_H
#define COMMON_DRIVERS_INTERFACES_CLOCKINTERFACE_H

#include <cstdint>

namespace drivers::interfaces {

/**
 * @brief Interface for accessing system or program monotonic time.
 *
 * This interface provides a consistent way to query system or program uptime.
 * It is intended to represent a **monotonic** time source that starts at
 * zero on program start and always moves forward (never goes backward or wraps).
 *
 * Time returned is typically used for scheduling, timeouts, profiling, etc.
 * Not suitable for wall-clock timestamps.
 */
class ClockInterface {
public:
    virtual ~ClockInterface()             = default;
    /**
     * @brief Get the system/program uptime in microseconds.
     *
     * @return Time in microseconds since the program/system started.
     */
    virtual uint64_t uptimeMicroseconds() = 0;

    /**
     * @briefet the system/program uptime in milliseconds.
     *
     * @return Time in milliseconds since the program/system started.
     */
    virtual uint64_t uptimeMilliseconds() = 0;

    /**
     * @brief et the system/program uptime in seconds.
     *
     * @return Time in seconds since the program/system started.
     */
    virtual uint64_t uptimeSeconds()      = 0;
};

}  // namespace drivers::interfaces

#endif  // COMMON_DRIVERS_INTERFACES_CLOCKINTERFACE_H
