#include "drivers/TimerDriver.h"

#include <hardware/irq.h>

#include <limits>

#include "assert/assert.h"

namespace drivers {

TimerDriver::TimerDriver(timer_hw_t* timer_instance, TimerAlarmChannel alarm_channel)
    : timer_instance_(timer_instance), alarm_channel_index_(static_cast<uint8_t>(alarm_channel)) {
    ASSERT(timer_instance_ != nullptr);
    ASSERT(alarm_channel_index_ < 4);

    // some handler already is put there by the pico stdlib which
    // hast to be removed, better thing would be to get rid of the interrupt handler automatic registering altogether
    irq_remove_handler(getIrqNumber(), irq_get_exclusive_handler(getIrqNumber()));
}

void TimerDriver::configureInMicroseconds(uint32_t period) { timer_period_us_ = period; }

void TimerDriver::configureInMilliseconds(uint32_t period) {
    [[maybe_unused]] uint64_t p = period * 1'000;
    ASSERT_WITH_MESSAGE(p < std::numeric_limits<uint32_t>::max(), "Too long period");

    configureInMicroseconds(period * 1'000);
}

void TimerDriver::configureInSeconds(uint32_t period) {
    [[maybe_unused]] uint64_t p = period * 1'000;
    ASSERT_WITH_MESSAGE(p < std::numeric_limits<uint32_t>::max(), "Too long period");

    configureInMilliseconds(period * 1'000);
}

void TimerDriver::start() {
    ASSERT(timer_period_us_ > 0);
    // TODO should this clear the interrupt or should the user be forced to call clearIrq
    // Clear the interrupt first if it’s pending
    timer_instance_->intr                        = (1u << alarm_channel_index_);

    timer_start_time_us_                         = timer_time_us_64(timer_instance_);
    uint32_t target_time_us                      = timer_start_time_us_ + timer_period_us_;

    // Set the alarm
    timer_instance_->alarm[alarm_channel_index_] = target_time_us;
    // Enable the interrupt for the alarm (the timer outputs 4 alarm irqs)
    timer_instance_->inte |= (1u << alarm_channel_index_);
}

void TimerDriver::stop() { timer_hardware_alarm_cancel(timer_instance_, alarm_channel_index_); }

void TimerDriver::restart() {
    ASSERT(timer_period_us_ > 0);

    // Move forward one period from previous start time
    timer_start_time_us_ += timer_period_us_;
    uint32_t target_time_us                      = timer_start_time_us_ + timer_period_us_;
    // TODO should this clear the interrupt or should the user be forced to call clearIrq
    // Clear the interrupt first if it’s pending
    timer_instance_->intr                        = (1u << alarm_channel_index_);
    timer_instance_->alarm[alarm_channel_index_] = target_time_us;

    // TODO should this even enable the interrupt so that user would be forced to call start on first run and then
    // interrupt is already enabled?
    timer_instance_->inte |= (1u << alarm_channel_index_);
}

bool TimerDriver::isRunning() const { return timer_instance_->armed & (1u << alarm_channel_index_); }

uint64_t TimerDriver::getElapsedMicroseconds() const {
    return timer_time_us_64(timer_instance_) - timer_start_time_us_;
}

uint64_t TimerDriver::getElapsedMilliseconds() const { return getElapsedMicroseconds() / 1'000; }

uint64_t TimerDriver::getElapsedSeconds() const { return getElapsedMilliseconds() / 1'000; }

void TimerDriver::forceFire() const {
    // Cancel the possible upcoming interrupt because it is forced to fire now
    timer_hardware_alarm_cancel(timer_instance_, alarm_channel_index_);
    timer_hardware_alarm_force_irq(timer_instance_, alarm_channel_index_);
}

unsigned int TimerDriver::getIrqNumber() const {
    return timer_hardware_alarm_get_irq_num(timer_instance_, alarm_channel_index_);
}

void TimerDriver::clearIrq() { timer_instance_->intr = (1u << alarm_channel_index_); }

}  // namespace drivers