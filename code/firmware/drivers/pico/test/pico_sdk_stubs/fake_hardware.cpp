// HOST TEST STUB: backing storage and behaviour for the simulated peripherals.
#include "fake_hardware.h"

#include <cstring>

#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/resets.h"
#include "hardware/structs/usb.h"
#include "hardware/structs/usb_dpram.h"
#include "pico/bootrom.h"
#include "pico/time.h"
#include "pico/unique_id.h"

namespace {

usb_hw_t g_usb_hw;      // the real register block
usb_hw_t g_usb_hw_set;  // "set" alias window
usb_hw_t g_usb_hw_clr;  // "clear" alias window
usb_device_dpram_t g_dpram;

irq_handler_t g_irq_handler = nullptr;
bool          g_irq_enabled = false;

bool g_gpio_level[64]    = {};
bool g_gpio_is_input[64] = {};
int  g_gpio_function[64] = {};

uint32_t g_clock_us        = 0;
uint32_t g_clock_auto_step = 1;

}  // namespace

usb_hw_t* const           usb_hw    = &g_usb_hw;
usb_device_dpram_t* const usb_dpram = &g_dpram;

// ---- address_mapped.h / alias windows ----
void* hw_set_alias_untyped(volatile void* addr) {
    return (addr == static_cast<volatile void*>(&g_usb_hw)) ? static_cast<void*>(&g_usb_hw_set) : nullptr;
}
void* hw_clear_alias_untyped(volatile void* addr) {
    return (addr == static_cast<volatile void*>(&g_usb_hw)) ? static_cast<void*>(&g_usb_hw_clr) : nullptr;
}
usb_stub_reg& usb_stub_reg::operator=(uint32_t v) {
    const char* self = reinterpret_cast<const char*>(this);
    auto inside = [self](const usb_hw_t& block) {
        const char* base = reinterpret_cast<const char*>(&block);
        return self >= base && self < base + sizeof(usb_hw_t);
    };
    if (inside(g_usb_hw_set)) {
        auto* target = reinterpret_cast<usb_stub_reg*>(reinterpret_cast<char*>(&g_usb_hw) + (self - reinterpret_cast<const char*>(&g_usb_hw_set)));
        target->value |= v;
    } else if (inside(g_usb_hw_clr)) {
        auto* target = reinterpret_cast<usb_stub_reg*>(reinterpret_cast<char*>(&g_usb_hw) + (self - reinterpret_cast<const char*>(&g_usb_hw_clr)));
        target->value &= ~v;
    } else {
        value = v;
    }
    return *this;
}

// ---- irq.h ----
void irq_set_exclusive_handler(unsigned, irq_handler_t h) { g_irq_handler = h; }
void irq_set_enabled(unsigned, bool enabled) { g_irq_enabled = enabled; }
bool irq_is_enabled(unsigned) { return g_irq_enabled; }

// ---- resets.h ----
void reset_block_mask(uint32_t) {}
void unreset_block_mask_wait_blocking(uint32_t) {}

// ---- gpio.h ----
void gpio_init(uint gpio) { g_gpio_is_input[gpio] = true; g_gpio_function[gpio] = GPIO_FUNC_SIO; }
void gpio_set_function(uint gpio, gpio_function_t fn) { g_gpio_function[gpio] = fn; }
void gpio_set_dir(uint gpio, bool out) { g_gpio_is_input[gpio] = !out; }
void gpio_pull_down(uint) {}
bool gpio_get(uint gpio) { return g_gpio_level[gpio]; }

// ---- unique_id.h ----
void pico_get_unique_board_id_string(char* id_out, uint len) {
    static const char K_ID[] = "E66138528B123456";
    strncpy(id_out, K_ID, len);
    if (len) id_out[len - 1] = '\0';
}

// ---- bootrom.h ----
void reset_usb_boot(uint32_t, uint32_t) { throw fake_hardware::BootloaderRequested(); }

// ---- time.h ----
uint32_t time_us_32(void) {
    const uint32_t now = g_clock_us;
    g_clock_us += g_clock_auto_step;
    return now;
}

// ---- controls ----
namespace fake_hardware {

void reset() {
    memset(&g_usb_hw, 0, sizeof(g_usb_hw));
    memset(&g_dpram, 0, sizeof(g_dpram));
    g_irq_handler = nullptr;
    g_irq_enabled = false;
    memset(g_gpio_level, 0, sizeof(g_gpio_level));
    memset(g_gpio_is_input, 0, sizeof(g_gpio_is_input));
    for (int& f : g_gpio_function) f = -1;
    g_clock_us        = 0;
    g_clock_auto_step = 1;
}

void setGpioInput(unsigned pin, bool level) { g_gpio_level[pin] = level; }
bool gpioIsInput(unsigned pin) { return g_gpio_is_input[pin]; }
int  gpioFunction(unsigned pin) { return g_gpio_function[pin]; }
void advanceClockUs(uint32_t us) { g_clock_us += us; }
void setClockAutoAdvanceUs(uint32_t us) { g_clock_auto_step = us; }
void (*installedIrqHandler())() { return g_irq_handler; }
bool irqEnabled() { return g_irq_enabled; }

}  // namespace fake_hardware
