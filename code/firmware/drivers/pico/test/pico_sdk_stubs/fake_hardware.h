#ifndef FAKEHARDWARE_H
#define FAKEHARDWARE_H
// HOST TEST STUB: state and controls for the simulated RP2 USB controller, GPIO, clock and ROM.
#include <cstdint>
#include <stdexcept>

namespace fake_hardware {

/** Thrown by the stubbed reset_usb_boot() so a test can observe the reboot request without exiting. */
struct BootloaderRequested : std::runtime_error {
    BootloaderRequested() : std::runtime_error("reset_usb_boot") {}
};

void reset();                                 // zero all fake peripherals
void setGpioInput(unsigned pin, bool level);  // drive a GPIO the driver reads
bool gpioIsInput(unsigned pin);
int  gpioFunction(unsigned pin);              // last gpio_set_function() value, -1 if never set
void advanceClockUs(uint32_t us);             // move the fake time_us_32() forward
void setClockAutoAdvanceUs(uint32_t us);      // per-read auto advance (so bounded waits terminate)
void (*installedIrqHandler())();              // whatever irq_set_exclusive_handler stored
bool irqEnabled();

}  // namespace fake_hardware

#endif
