#ifndef FIRMWARE_DRIVERS_PICO_TIMERDRIVER_H
#define FIRMWARE_DRIVERS_PICO_TIMERDRIVER_H

#include <hardware/timer.h>  // PICO SDK

#include <cstdint>

#include "drivers/interfaces/TimerInterface.h"

namespace drivers {

/**
 * @brief Enum representing the available alarm channels on a hardware timer.
 */
enum class TimerAlarmChannel : uint8_t { alarm0 = 0, alarm1 = 1, alarm2 = 2, alarm3 = 3 };

/**
 * @brief Driver class for controlling the hardware timer and its alarm channels.
 *
 * The system timer peripheral provides a microsecond timebase for the system, and generates interrupts
 * based on this timebase. Timer consists of a single 64-bit counter, incrementing once per microsecond and four alarms
 * that match on the lower 32 bits of the counter and generate IRQ on match.
 *
 * This driver configures and controls one of the hardware timer's four alarm channels (alarm 0–3).
 * It provides functionality to configure the timer period, start and stop the timer, and handle
 * interrupt triggering manually or via hardware match events.
 *
 * The maximum delay that can be scheduled is limited to 2^32 microseconds (~4295 seconds), due to the 32-bit
 * comparison.
 */
class TimerDriver final : public interfaces::TimerInterface {
public:
    /**
     * @brief Construct a new TimerDriver object.
     *
     * @param timer_instance Pointer to the hardware timer peripheral (`timer_hw` or `timer1_hw`)
     * @param alarm_channel  Hardware alarm channel to use (0–3)
     */
     TimerDriver(timer_hw_t* timer_instance, TimerAlarmChannel alarm_channel);
    ~TimerDriver() override = default;

    /**
     * @brief Set the timer period in microseconds.
     *
     * This does not start the timer. Use `start()` after configuring.
     *
     * @param period Duration in microseconds
     */
    void configureInMicroseconds(uint32_t period) override;
    /**
     * @brief Set the timer period in milliseconds.
     *
     *  This does not start the timer. Use `start()` after configuring.
     *
     * @param period Duration in milliseconds
     */
    void configureInMilliseconds(uint32_t period) override;
    /**
     * @brief Set the timer period in seconds.
     *
     *  This does not start the timer. Use `start()` after configuring.
     *
     * @param period Duration in seconds
     */
    void configureInSeconds(uint32_t period) override;

    /**
     * Start the timer using the configured period
     *
     * If the timer is already running,
     * this will reset and rearm it using the configured period from now.
     */
    void start() override;
    /**
     * @brief Stop the timer by canceling the current alarm.
     */
    void stop() override;
    /**
     * @brief Restart the timer using the last scheduled period.
     *
     * This is useful for periodic timers to continue ticking without drift.
     * Unlike `start()`, this continues from the last scheduled time.
     */
    void restart() override;

    /**
     * @brief Check whether the timer is currently running.
     *
     * Does the check by checking if the hardware alarm is currently alarmed.
     *
     * @return true if the timer is running, false otherwise
     */
    [[nodiscard]] bool isRunning() const override;

    /**
     * @brief Get the time elapsed since the timer started in microseconds.
     *
     * @return Elapsed time in µs
     */
    [[nodiscard]] uint64_t getElapsedMicroseconds() const override;
    /**
     * @brief Get the time elapsed since the timer started in milliseconds.
     *
     * @return Elapsed time in ms
     */
    [[nodiscard]] uint64_t getElapsedMilliseconds() const override;
    /**
     * @brief Get the time elapsed since the timer started in seconds.
     *
     * @return Elapsed time in s
     */
    [[nodiscard]] uint64_t getElapsedSeconds() const override;

    /**
     * @brief Force the timer to trigger its interrupt immediately.
     */
    void forceFire() const override;

    /**
     * @brief Get the NVIC interrupt number associated with the configured alarm.
     *
     * @return IRQ number
     */
    [[nodiscard]] unsigned int getIrqNumber() const;
    /**
     * @brief Clear the interrupt flag for the configured alarm channel.
     *
     * This must be called inside the interrupt service routine (ISR) to acknowledge
     * and clear the interrupt. Failing to do so will cause the ISR to be immediately
     * retriggered after returning.
     *
     * The timer can also be started or restarted in the isr which will clear the intrreupt intrnally as well.
     */
    void clearIrq();

private:
    timer_hw_t* timer_instance_;
    uint8_t     alarm_channel_index_;
    uint32_t    timer_period_us_;
    uint64_t    timer_start_time_us_;
};

}  // namespace drivers

#endif  // FIRMWARE_DRIVERS_PICO_TIMERDRIVER_H
