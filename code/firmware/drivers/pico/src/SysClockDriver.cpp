#include "drivers/SysClockDriver.h"

#include <hardware/timer.h>

namespace drivers {

uint64_t SysClockDriver::uptimeMicroseconds() { return timer_time_us_64(timer_get_instance(PICO_DEFAULT_TIMER)); }

uint64_t SysClockDriver::uptimeMilliseconds() { return uptimeMicroseconds() / 1000; }

uint64_t SysClockDriver::uptimeSeconds() { return uptimeMilliseconds() / 1000; }

}  // namespace drivers