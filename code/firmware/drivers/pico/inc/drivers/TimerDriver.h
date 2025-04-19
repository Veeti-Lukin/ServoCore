#ifndef FIRMWARE_DRIVERS_PICO_TIMERDRIVER_H
#define FIRMWARE_DRIVERS_PICO_TIMERDRIVER_H

#include <hardware/timer.h>  // PICO SDK

#include <cstdint>

namespace drivers {

/*
enum class TimerMode {
    one_shot,
    periodic,
    free_running,
};*/

enum class TimerAlarmChannel : uint8_t { alarm1 = 0, alarm2 = 1, alarm3 = 2, alarm4 = 3 };

class TimerDriver {
public:
     TimerDriver(timer_hw_t* timer_instance, TimerAlarmChannel alarm_channel);
    ~TimerDriver() = default;

    void configureInMicroseconds(uint32_t interval);
    void configureInMilliseconds(uint32_t interval);
    void configureInSeconds(uint32_t interval);

    void start();
    void stop();
    void restart();

    [[nodiscard]] bool isRunning() const;

    [[nodiscard]] uint64_t getElapsedMicroseconds() const;
    [[nodiscard]] uint64_t getElapsedMilliseconds() const;
    [[nodiscard]] uint64_t getElapsedSeconds() const;

    void forceTrigger() const;

    unsigned int getIrqNumber() const;

private:
    timer_hw_t* timer_instance_;
    uint8_t     alarm_channel_index_;
    uint64_t    timer_period_us_;
    uint64_t    timer_start_time_;

    void enableInterrupt() const;
    void disableInterrupt() const;
};

}  // namespace drivers

#endif  // FIRMWARE_DRIVERS_PICO_TIMERDRIVER_H
