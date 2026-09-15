#ifndef PICO_BOOTROM_H
#define PICO_BOOTROM_H
// HOST TEST STUB: throws fake_hardware::BootloaderRequested instead of rebooting.
#include <cstdint>
[[noreturn]] void reset_usb_boot(uint32_t usb_activity_gpio_pin_mask, uint32_t disable_interface_mask);
#endif
