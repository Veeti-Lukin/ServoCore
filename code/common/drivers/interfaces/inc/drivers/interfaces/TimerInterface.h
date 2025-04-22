#ifndef COMMON_DRIVERS_INTERFACES_TIMERINTERFACE_H
#define COMMON_DRIVERS_INTERFACES_TIMERINTERFACE_H

#include <cstdint>

namespace drivers::interfaces {
/**
 * @brief Abstract interface for a hardware or software timer driver.
 *
 * This interface provides a unified way to control timers across different
 * implementations. It supports millisecond, microsecond, and second configuration granularity.
 * It also provides basic time tracking and manual firing.
 *
 * The interface models a timer which runs once and fires after which it has to be re-armed
 * with start (or more preferably restart to reduce drift) if the timer needs to run multiple times.
 */
class TimerInterface {
public:
    virtual ~TimerInterface()                                     = default;

    /**
     * @brief Set the timer period in microseconds.
     *
     * This does not start the timer. Use `start()` after configuring.
     *
     * @param period Duration in microseconds
     */
    virtual void configureInMicroseconds(uint32_t period)         = 0;
    /**
     * @brief Set the timer period in milliseconds.
     *
     *  This does not start the timer. Use `start()` after configuring.
     *
     * @param period Duration in milliseconds
     */
    virtual void configureInMilliseconds(uint32_t period)         = 0;
    /**
     * @brief Set the timer period in seconds.
     *
     *  This does not start the timer. Use `start()` after configuring.
     *
     * @param period Duration in seconds
     */
    virtual void configureInSeconds(uint32_t period)              = 0;

    /**
     * Start the timer using the configured period
     *
     * If the timer is already running,
     * this will reset and rearm it using the configured period from now.
     */
    virtual void start()                                          = 0;
    /**
     * Stop or disarm the timer
     */
    virtual void stop()                                           = 0;
    // TODO Rename to periodicRestart?
    /**
     * @brief Restart the timer using the last scheduled period.
     *
     * This is useful for periodic timers to continue ticking without drift.
     * Unlike `start()`, this continues from the last scheduled time.
     */
    virtual void restart()                                        = 0;

    /**
     * @brief Check whether the timer is currently running.
     *
     * @return true if the timer is running, false otherwise
     */
    [[nodiscard]] virtual bool isRunning() const                  = 0;

    /**
     * @brief Get the time elapsed since the timer started in microseconds.
     *
     * @return Elapsed time in µs
     */
    [[nodiscard]] virtual uint64_t getElapsedMicroseconds() const = 0;
    /**
     * @brief Get the time elapsed since the timer started in milliseconds.
     *
     * @return Elapsed time in ms
     */
    [[nodiscard]] virtual uint64_t getElapsedMilliseconds() const = 0;
    /**
     * @brief Get the time elapsed since the timer started in seconds.
     *
     * @return Elapsed time in s
     */
    [[nodiscard]] virtual uint64_t getElapsedSeconds() const      = 0;

    /**
     * @brief Force the timer to fire immediately.
     *
     * If timer is currently running cancels the upcoming alarm.
     */
    virtual void forceFire() const                                = 0;
};

}  // namespace drivers::interfaces
#endif  // COMMON_DRIVERS_INTERFACES_TIMERINTERFACE_H
