#include "drivers/TimerDriver.h"

#include <hardware/irq.h>

#include "assert/assert.h"

namespace drivers {

TimerDriver::TimerDriver(timer_hw_t* timer_instance, TimerAlarmChannel alarm_channel)
    : timer_instance_(timer_instance), alarm_channel_index_(static_cast<uint8_t>(alarm_channel)) {
    ASSERT(timer_instance_ != nullptr);
    ASSERT(alarm_channel_index_ < 4);
}

void TimerDriver::configureInMicroseconds(uint32_t interval) { timer_period_us_ = interval; }

void TimerDriver::configureInMilliseconds(uint32_t interval) { configureInMicroseconds(interval * 1'000); }

void TimerDriver::configureInSeconds(uint32_t interval) { configureInMilliseconds(interval * 1'000); }

void TimerDriver::start() {
    // Clear the interrupt first if it’s pending
    hw_clear_bits(&timer_instance_->intr, 1u << alarm_channel_index_);
    timer_start_time_ = timer_time_us_64(timer_instance_);
    timer_hardware_alarm_set_target(timer_instance_, alarm_channel_index_, timer_start_time_ + timer_period_us_);
}

void TimerDriver::stop() { timer_hardware_alarm_cancel(timer_instance_, alarm_channel_index_); }

void TimerDriver::restart() {}

bool TimerDriver::isRunning() const { return timer_instance_->armed & (1u << alarm_channel_index_); }

uint64_t TimerDriver::getElapsedMicroseconds() const { return timer_time_us_64(timer_instance_) - timer_start_time_; }

uint64_t TimerDriver::getElapsedMilliseconds() const { return getElapsedMicroseconds() / 1'000; }

uint64_t TimerDriver::getElapsedSeconds() const { return getElapsedMilliseconds() / 1'000; }

void TimerDriver::forceTrigger() const { timer_hardware_alarm_force_irq(timer_instance_, alarm_channel_index_); }

unsigned int TimerDriver::getIrqNumber() const {
    return timer_hardware_alarm_get_irq_num(timer_instance_, alarm_channel_index_);
}

void TimerDriver::enableInterrupt() const { irq_set_enabled(getIrqNumber(), true); }

void TimerDriver::disableInterrupt() const { irq_set_enabled(getIrqNumber(), false); }

}  // namespace drivers