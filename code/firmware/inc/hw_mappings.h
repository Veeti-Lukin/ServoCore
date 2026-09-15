#ifndef HW_MAPPINGS_H
#define HW_MAPPINGS_H

#include <drivers/TimerDriver.h>
#include <drivers/usb/UsbCdcChannel.h>
#include <drivers/usb/UsbDevice.h>
#include <hardware/timer.h>
#include <hardware/uart.h>

#include <cstddef>

namespace hw_mappings {

const auto     K_SERIAL_COMMUNICATION_UART_INSTANCE = uart0;
constexpr auto K_SERIAL_COMMUNICATION_UART_TX_PIN   = 0;
constexpr auto K_SERIAL_COMMUNICATION_UART_RX_PIN   = 1;

const auto     K_DEBUG_UART_INSTANCE                = uart1;  // Cant be constexpr
constexpr auto K_DEBUG_UART_TX_PIN                  = 4;
constexpr auto K_DEBUG_UART_RX_PIN                  = 5;

constexpr unsigned int K_STATUS_LED_RED_PIN         = 15;
constexpr unsigned int K_STATUS_LED_GREEN_PIN       = 9;
constexpr unsigned int K_STATUS_LED_BLUE_PIN        = 13;

const auto     K_PERIODIC_LED_TIMER_INSTANCE        = timer_hw;
constexpr auto K_PERIODIC_LED_TIMER_ALARM_CHANNEL   = drivers::TimerAlarmChannel::alarm3;

// ---------------------------------- USB ----------------------------------
// TODO: TEMPORARY identity. This is the Pico SDK's own CDC VID/PID so every host already knows the device while
// the real ones are pending. Replace with ServoCore's VID/PID before anything leaves the bench.
constexpr drivers::usb::DeviceConfig K_USB_CONFIG{
    .vendor_id          = 0x2E8A,
    .product_id         = 0x000A,
    .device_version_bcd = 0x0100,
    .self_powered       = true,  // MCU runs from the motor supply through the buck, not from VBUS
    .max_power_ma       = 100,
};

// GPIO wired to the USB-C VBUS divider, or -1 when the board has none. The Pico W routes VBUS sense to its
// wireless chip, so there is nothing to read on the dev board. On the ServoCore board use one of GPIO 1, 4, 7,
// ... 28 so the controller gates the D+ pull-up itself (see drivers::usb::UsbDeviceBase::VbusSense).
constexpr int K_USB_VBUS_DETECT_PIN = -1;

// The USB device this board presents: two virtual serial ports, protocol first, debug second. Interface numbers,
// endpoints and descriptors are derived from this type list at compile time.
using UsbDevice = drivers::usb::UsbDevice<K_USB_CONFIG, drivers::usb::Cdc<256, 256>, drivers::usb::Cdc<256, 256>>;
constexpr size_t K_USB_PROTOCOL_CHANNEL = 0;
constexpr size_t K_USB_DEBUG_CHANNEL    = 1;

}  // namespace hw_mappings

#endif  // HW_MAPPINGS_H
