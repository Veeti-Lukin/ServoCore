#ifndef USBDEVICE_H
#define USBDEVICE_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <tuple>

#include "drivers/usb/UsbDeviceBase.h"
#include "drivers/usb/usb_descriptor_plan.h"

namespace drivers::usb {

/**
 * @brief A USB device composed from a compile-time list of functions.
 *
 * The device's structure (interfaces, endpoints, DPRAM layout, device and configuration descriptors) is derived
 * from the type list by DevicePlan and lives in flash as constexpr tables. Only the human-readable strings are
 * supplied at runtime.
 *
 * Usage (from hw_mappings.h and main.cpp):
 * @code
 *   constexpr drivers::usb::DeviceConfig K_USB_CONFIG{.vendor_id = 0x1234, .product_id = 0x0001};
 *
 *   using UsbDevice = drivers::usb::UsbDevice<K_USB_CONFIG, drivers::usb::Cdc<256, 256>, drivers::usb::Cdc<256, 256>>;
 *   UsbDevice usb_device({.manufacturer = "Veeti Lukin", .product = "ServoCore",
 *                         .function_names = {"ServoCore Protocol", "ServoCore Debug"}});
 *
 *   auto& usb_protocol = usb_device.channel<0>();   // a BufferedSerialCommunicationInterface
 *   auto& usb_debug    = usb_device.channel<1>();
 *
 *   // in initHW(): install usb_device.handleInterrupt() on UsbDevice::K_NVIC_INTERRUPT_NUMBER, then usb_device.init()
 *   // in the main loop: usb_device.run()
 * @endcode
 *
 * @tparam config    Board facts baked into the descriptors.
 * @tparam Functions Function types (e.g. Cdc<tx, rx>), in interface order.
 */
template <DeviceConfig config, UsbFunction... Functions>
class UsbDevice final : public UsbDeviceBase {
public:
    using Plan = DevicePlan<config, Functions...>;

    /** @brief Human-readable strings. Everything else about the device is compile time. */
    struct Strings {
        const char* manufacturer;
        const char* product;
        /// nullptr requests the chip's unique ID, so several boards can be told apart on one host.
        const char* serial = nullptr;
        /// One name per function string, in function order (a Cdc has exactly one: the port name).
        std::array<const char*, Plan::K_NUM_FUNCTION_STRINGS> function_names;
    };

    /**
     * @param strings         Device and function names.
     * @param vbus_detect_pin GPIO wired to a VBUS divider, or -1 if the board has no VBUS sense.
     */
    explicit UsbDevice(const Strings& strings, int vbus_detect_pin = -1);

    UsbDevice(const UsbDevice&)            = delete;
    UsbDevice& operator=(const UsbDevice&) = delete;

    /** @brief The runtime object of the index-th function in the type list. */
    template <size_t index>
    [[nodiscard]] auto& channel() {
        static_assert(index < Plan::K_NUM_FUNCTIONS, "No such function in this device");
        return std::get<index>(channels_);
    }

private:
    std::tuple<typename Functions::Channel...>          channels_;
    std::array<EndpointState, Plan::K_NUM_ENDPOINTS>    endpoint_states_{};
    std::array<UsbFunctionBase*, Plan::K_NUM_FUNCTIONS> function_pointers_{};
    std::array<const char*, Plan::K_NUM_STRINGS>        strings_{};
};

/// ------------------------ DEFINITIONS --------------------------------------

template <DeviceConfig config, UsbFunction... Functions>
UsbDevice<config, Functions...>::UsbDevice(const Strings& strings, int vbus_detect_pin)
    : UsbDeviceBase(vbus_detect_pin) {
    // Function pointers for the core's dispatch, in type-list order.
    std::apply(
        [this](auto&... channel) {
            size_t i = 0;
            ((function_pointers_[i++] = &channel), ...);
        },
        channels_);

    // String table: index 0 is the language table (built on demand), 1..3 device strings, then functions.
    strings_[Plan::K_STRING_INDEX_MANUFACTURER] = strings.manufacturer;
    strings_[Plan::K_STRING_INDEX_PRODUCT]      = strings.product;
    strings_[Plan::K_STRING_INDEX_SERIAL]       = strings.serial;
    for (size_t i = 0; i < Plan::K_NUM_FUNCTION_STRINGS; i++) {
        strings_[Plan::K_FIRST_FUNCTION_STRING_INDEX + i] = strings.function_names[i];
    }

    attachTables(Tables{
        .endpoint_plans           = Plan::K_ENDPOINTS,
        .endpoint_states          = endpoint_states_,
        .functions                = function_pointers_,
        .device_descriptor        = Plan::K_DEVICE_DESCRIPTOR,
        .configuration_descriptor = Plan::K_CONFIGURATION_DESCRIPTOR,
        .strings                  = strings_,
        .self_powered             = config.self_powered,
        .bootloader_touch_enabled = config.bootloader_touch_enabled,
        .tx_block_timeout_us      = config.tx_block_timeout_us,
    });

    // Give every channel its numbers from the plan.
    std::apply(
        [this](auto&... channel) {
            size_t i = 0;
            ((channel.bind(*this, static_cast<uint8_t>(i), Plan::K_LAYOUTS[i].first_interface), i++), ...);
        },
        channels_);
}

}  // namespace drivers::usb

#endif  // USBDEVICE_H
