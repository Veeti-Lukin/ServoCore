#ifndef USBDESCRIPTORPLAN_H
#define USBDESCRIPTORPLAN_H

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>

#include "drivers/usb/usb_protocol.h"

/**
 * @file usb_descriptor_plan.h
 * @brief Compile-time composition of a USB device from a list of function types.
 *
 * Given a DeviceConfig and a parameter pack of function types (e.g. two Cdc functions), DevicePlan computes at
 * compile time:
 *   - interface numbers, string indices and endpoint addresses for every function,
 *   - the endpoint table with each endpoint's DPRAM buffer offset,
 *   - the complete device descriptor and configuration descriptor as constexpr byte arrays.
 *
 * Nothing in the descriptors is written by hand: adding a function to the type list is the only change needed.
 * Structural invariants (endpoint counts, DPRAM budget, descriptor length fields) are checked with static_assert.
 *
 * This header has no hardware dependency and is compiled unchanged in the host-side tests.
 */
namespace drivers::usb {

/// ------------------------------ Hardware facts of the RP2040 / RP2350 USB controller ------------------------------

/// Total size of the USB dual-port RAM shared with the controller.
constexpr uint16_t K_DPRAM_SIZE                 = 4096;
/// First DPRAM byte available for endpoint data buffers (after setup packet, control registers and EP0 buffers).
constexpr uint16_t K_DPRAM_FIRST_BUFFER_OFFSET  = 0x180;
/// Every non-control endpoint gets one single-buffered 64-byte packet buffer.
constexpr uint16_t K_ENDPOINT_BUFFER_SIZE       = 64;
/// Endpoint numbers 1..15 are available per direction; endpoint 0 is the control endpoint.
constexpr uint8_t K_MAX_ENDPOINTS_PER_DIRECTION = 15;

/// ------------------------------ Types shared between plan and functions ------------------------------

/**
 * @brief Board-level facts baked into the descriptors. Passed as a non-type template parameter, so it must stay a
 *        structural type (public integral/bool members only).
 */
struct DeviceConfig {
    uint16_t vendor_id;
    uint16_t product_id;
    uint16_t device_version_bcd   = 0x0100;
    /// Powered from the board's own supply rather than from VBUS.
    bool self_powered             = true;
    /// Current drawn from VBUS, reported to the host in 2 mA units. Keep at 100 or below for a self-powered device.
    uint16_t max_power_ma         = 100;
    /// Reboot into the ROM bootloader when a host sets 1200 baud and then drops DTR (the Arduino convention).
    bool bootloader_touch_enabled = true;
    /// How long transmit may wait for space in a full TX ring while the host is connected, before dropping.
    uint32_t tx_block_timeout_us  = 5000;
};

/** @brief One endpoint a function needs, before numbers are assigned. */
struct EndpointSpec {
    uint8_t  direction;      // K_ENDPOINT_DIRECTION_IN / _OUT
    uint8_t  transfer_type;  // K_TRANSFER_TYPE_*
    uint16_t max_packet_size;
};

/** @brief One endpoint after the plan has assigned it a number and a DPRAM buffer. */
struct EndpointPlan {
    uint8_t  address;  // endpoint number with the direction bit
    uint8_t  transfer_type;
    uint16_t max_packet_size;
    uint16_t dpram_offset;       // offset of this endpoint's 64-byte buffer inside DPRAM
    uint8_t  function_index;     // which function in the type list owns it
    uint8_t  index_in_function;  // position in that function's K_ENDPOINTS

    [[nodiscard]] constexpr bool    isIn() const { return (address & K_ENDPOINT_DIRECTION_IN) != 0; }
    [[nodiscard]] constexpr uint8_t number() const { return address & 0x0F; }
};

constexpr size_t K_MAX_ENDPOINTS_PER_FUNCTION = 4;

/** @brief What the plan hands to a function so it can write its own slice of the configuration descriptor. */
struct FunctionLayout {
    uint8_t first_interface;
    uint8_t first_string_index;
    /// Resolved addresses, in the same order as the function's K_ENDPOINTS.
    std::array<uint8_t, K_MAX_ENDPOINTS_PER_FUNCTION> endpoint_addresses;
};

/**
 * @brief Requirements on a function type so it can be composed into a DevicePlan.
 *
 * A function declares its footprint as constants and provides a constexpr descriptor builder. Its Channel type is
 * the runtime object the device instantiates for it.
 */
template <typename F>
concept UsbFunction = requires(FunctionLayout layout) {
    { F::K_NUM_INTERFACES } -> std::convertible_to<uint8_t>;
    { F::K_NUM_STRINGS } -> std::convertible_to<uint8_t>;
    { F::K_DESCRIPTOR_SIZE } -> std::convertible_to<uint16_t>;
    { F::K_ENDPOINTS.size() } -> std::convertible_to<size_t>;
    { F::buildDescriptor(layout) } -> std::same_as<std::array<uint8_t, F::K_DESCRIPTOR_SIZE>>;
    typename F::Channel;
};

/// ------------------------------ Builders ------------------------------
namespace detail {

template <UsbFunction... Functions>
constexpr size_t totalEndpoints() {
    return (size_t{0} + ... + Functions::K_ENDPOINTS.size());
}

template <UsbFunction... Functions>
constexpr auto computeEndpoints() {
    std::array<EndpointPlan, totalEndpoints<Functions...>()> out{};
    size_t                                                   next           = 0;
    uint8_t                                                  next_in        = 1;
    uint8_t                                                  next_out       = 1;
    uint16_t                                                 next_offset    = K_DPRAM_FIRST_BUFFER_OFFSET;
    uint8_t                                                  function_index = 0;

    auto place                                                              = [&](const auto& specs) {
        for (size_t k = 0; k < specs.size(); k++) {
            const EndpointSpec& spec   = specs[k];
            const bool          is_in  = spec.direction == K_ENDPOINT_DIRECTION_IN;
            const uint8_t       number = is_in ? next_in++ : next_out++;
            out[next++]                = EndpointPlan{
                .address           = static_cast<uint8_t>(number | spec.direction),
                .transfer_type     = spec.transfer_type,
                .max_packet_size   = spec.max_packet_size,
                .dpram_offset      = next_offset,
                .function_index    = function_index,
                .index_in_function = static_cast<uint8_t>(k),
            };
            next_offset = static_cast<uint16_t>(next_offset + K_ENDPOINT_BUFFER_SIZE);
        }
        function_index++;
    };
    (place(Functions::K_ENDPOINTS), ...);
    return out;
}

template <UsbFunction... Functions>
constexpr auto computeLayouts(const std::array<EndpointPlan, totalEndpoints<Functions...>()>& endpoints,
                              uint8_t first_function_string_index) {
    constexpr std::array<uint8_t, sizeof...(Functions)> num_interfaces = {Functions::K_NUM_INTERFACES...};
    constexpr std::array<uint8_t, sizeof...(Functions)> num_strings    = {Functions::K_NUM_STRINGS...};

    std::array<FunctionLayout, sizeof...(Functions)> out{};
    uint8_t                                          interface = 0;
    uint8_t                                          string    = first_function_string_index;
    for (size_t f = 0; f < sizeof...(Functions); f++) {
        out[f].first_interface    = interface;
        out[f].first_string_index = string;
        interface                 = static_cast<uint8_t>(interface + num_interfaces[f]);
        string                    = static_cast<uint8_t>(string + num_strings[f]);
        for (const EndpointPlan& ep : endpoints) {
            if (ep.function_index == f) {
                out[f].endpoint_addresses[ep.index_in_function] = ep.address;
            }
        }
    }
    return out;
}

template <DeviceConfig config, UsbFunction... Functions>
constexpr auto buildConfigurationDescriptor(const std::array<FunctionLayout, sizeof...(Functions)>& layouts) {
    constexpr uint16_t K_TOTAL_LENGTH   = static_cast<uint16_t>(9 + (uint16_t{0} + ... + Functions::K_DESCRIPTOR_SIZE));
    constexpr uint8_t  K_NUM_INTERFACES = static_cast<uint8_t>((uint8_t{0} + ... + Functions::K_NUM_INTERFACES));

    std::array<uint8_t, K_TOTAL_LENGTH> out{};
    size_t                              pos = 0;

    // Configuration descriptor header (9 bytes).
    out[pos++]                              = 9;
    out[pos++]                              = K_DESCRIPTOR_TYPE_CONFIGURATION;
    out[pos++]                              = static_cast<uint8_t>(K_TOTAL_LENGTH & 0xFF);
    out[pos++]                              = static_cast<uint8_t>(K_TOTAL_LENGTH >> 8);
    out[pos++]                              = K_NUM_INTERFACES;
    out[pos++]                              = 1;                                               // bConfigurationValue
    out[pos++]                              = 0;                                               // iConfiguration
    out[pos++]            = static_cast<uint8_t>(0x80 | (config.self_powered ? 0x40 : 0x00));  // bmAttributes
    out[pos++]            = static_cast<uint8_t>(config.max_power_ma / 2);                     // bMaxPower

    size_t function_index = 0;
    auto   append         = [&](const auto& descriptor) {
        for (uint8_t byte : descriptor) {
            out[pos++] = byte;
        }
    };
    (append(Functions::buildDescriptor(layouts[function_index++])), ...);
    return out;
}

template <DeviceConfig config>
constexpr std::array<uint8_t, 18> buildDeviceDescriptor(uint8_t manufacturer_string, uint8_t product_string,
                                                        uint8_t serial_string) {
    return {
        18,                        // bLength
        K_DESCRIPTOR_TYPE_DEVICE,  // bDescriptorType
        0x00,
        0x02,                                                // bcdUSB = 2.00
        K_DEVICE_CLASS_MISCELLANEOUS,                        // bDeviceClass    } composite device described by
        K_DEVICE_SUBCLASS_COMMON,                            // bDeviceSubClass } Interface Association Descriptors
        K_DEVICE_PROTOCOL_IAD,                               // bDeviceProtocol }
        static_cast<uint8_t>(K_FULL_SPEED_MAX_PACKET_SIZE),  // bMaxPacketSize0
        static_cast<uint8_t>(config.vendor_id & 0xFF),
        static_cast<uint8_t>(config.vendor_id >> 8),
        static_cast<uint8_t>(config.product_id & 0xFF),
        static_cast<uint8_t>(config.product_id >> 8),
        static_cast<uint8_t>(config.device_version_bcd & 0xFF),
        static_cast<uint8_t>(config.device_version_bcd >> 8),
        manufacturer_string,
        product_string,
        serial_string,
        1,  // bNumConfigurations
    };
}

}  // namespace detail

/// ------------------------------ The plan ------------------------------

/**
 * @brief Everything about the device's structure that is known at compile time.
 * @tparam config    Board facts (VID/PID, power).
 * @tparam Functions The functions the device is composed of, in interface order.
 */
template <DeviceConfig config, UsbFunction... Functions>
struct DevicePlan {
    static constexpr DeviceConfig K_CONFIG    = config;

    static constexpr size_t  K_NUM_FUNCTIONS  = sizeof...(Functions);
    static constexpr uint8_t K_NUM_INTERFACES = static_cast<uint8_t>((uint8_t{0} + ... + Functions::K_NUM_INTERFACES));
    static constexpr size_t  K_NUM_ENDPOINTS  = detail::totalEndpoints<Functions...>();
    static constexpr size_t  K_NUM_FUNCTION_STRINGS        = (size_t{0} + ... + Functions::K_NUM_STRINGS);

    /// String descriptor indices. 0 is the language table; 1..3 are the device strings; functions follow.
    static constexpr uint8_t K_STRING_INDEX_MANUFACTURER   = 1;
    static constexpr uint8_t K_STRING_INDEX_PRODUCT        = 2;
    static constexpr uint8_t K_STRING_INDEX_SERIAL         = 3;
    static constexpr uint8_t K_FIRST_FUNCTION_STRING_INDEX = 4;
    static constexpr size_t  K_NUM_STRINGS                 = K_FIRST_FUNCTION_STRING_INDEX + K_NUM_FUNCTION_STRINGS;

    static constexpr std::array<EndpointPlan, K_NUM_ENDPOINTS>   K_ENDPOINTS = detail::computeEndpoints<Functions...>();
    static constexpr std::array<FunctionLayout, K_NUM_FUNCTIONS> K_LAYOUTS =
        detail::computeLayouts<Functions...>(K_ENDPOINTS, K_FIRST_FUNCTION_STRING_INDEX);

    static constexpr auto K_CONFIGURATION_DESCRIPTOR =
        detail::buildConfigurationDescriptor<config, Functions...>(K_LAYOUTS);
    static constexpr auto K_DEVICE_DESCRIPTOR = detail::buildDeviceDescriptor<config>(
        K_STRING_INDEX_MANUFACTURER, K_STRING_INDEX_PRODUCT, K_STRING_INDEX_SERIAL);

    /// Number of endpoints in each direction, for the static checks below and for runtime bookkeeping.
    static constexpr uint8_t K_NUM_IN_ENDPOINTS = [] {
        uint8_t n = 0;
        for (const EndpointPlan& ep : K_ENDPOINTS) n = static_cast<uint8_t>(n + (ep.isIn() ? 1 : 0));
        return n;
    }();
    static constexpr uint8_t K_NUM_OUT_ENDPOINTS = static_cast<uint8_t>(K_NUM_ENDPOINTS - K_NUM_IN_ENDPOINTS);

    /// ------------------------------ Structural checks ------------------------------
    static_assert(K_NUM_FUNCTIONS > 0, "A USB device needs at least one function");
    static_assert(K_NUM_IN_ENDPOINTS <= K_MAX_ENDPOINTS_PER_DIRECTION, "Too many IN endpoints for the controller");
    static_assert(K_NUM_OUT_ENDPOINTS <= K_MAX_ENDPOINTS_PER_DIRECTION, "Too many OUT endpoints for the controller");
    static_assert(K_DPRAM_FIRST_BUFFER_OFFSET + K_NUM_ENDPOINTS * K_ENDPOINT_BUFFER_SIZE <= K_DPRAM_SIZE,
                  "Endpoint buffers do not fit in USB DPRAM");
    static_assert(K_NUM_STRINGS <= 255, "Too many string descriptors");
    static_assert(((Functions::K_ENDPOINTS.size() <= K_MAX_ENDPOINTS_PER_FUNCTION) && ...),
                  "A function declares more endpoints than FunctionLayout can carry");
    static_assert(K_CONFIGURATION_DESCRIPTOR[2] == (K_CONFIGURATION_DESCRIPTOR.size() & 0xFF) &&
                      K_CONFIGURATION_DESCRIPTOR[3] == (K_CONFIGURATION_DESCRIPTOR.size() >> 8),
                  "wTotalLength does not match the built configuration descriptor");
    static_assert(K_CONFIGURATION_DESCRIPTOR[4] == K_NUM_INTERFACES, "bNumInterfaces does not match the function list");
    static_assert(config.max_power_ma <= 500, "bMaxPower cannot represent more than 500 mA");
    static_assert(!config.self_powered || config.max_power_ma <= 100,
                  "A self-powered device should declare at most 100 mA from VBUS");
};

}  // namespace drivers::usb

#endif  // USBDESCRIPTORPLAN_H
