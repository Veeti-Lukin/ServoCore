#ifndef USBPROTOCOL_H
#define USBPROTOCOL_H

#include <cstdint>

/**
 * @file usb_protocol.h
 * @brief USB 2.0 and CDC ACM protocol constants and packed on-the-wire structures.
 *
 * Everything in this file comes straight from the USB 2.0 and CDC 1.2 specifications and has no dependency on
 * any hardware. It is shared by the compile-time descriptor builder, the runtime device and the host-side tests.
 */
namespace drivers::usb {

/// ------------------------------ Descriptor types ------------------------------
constexpr uint8_t K_DESCRIPTOR_TYPE_DEVICE                = 0x01;
constexpr uint8_t K_DESCRIPTOR_TYPE_CONFIGURATION         = 0x02;
constexpr uint8_t K_DESCRIPTOR_TYPE_STRING                = 0x03;
constexpr uint8_t K_DESCRIPTOR_TYPE_INTERFACE             = 0x04;
constexpr uint8_t K_DESCRIPTOR_TYPE_ENDPOINT              = 0x05;
constexpr uint8_t K_DESCRIPTOR_TYPE_DEVICE_QUALIFIER      = 0x06;
constexpr uint8_t K_DESCRIPTOR_TYPE_INTERFACE_ASSOCIATION = 0x0B;
constexpr uint8_t K_DESCRIPTOR_TYPE_CS_INTERFACE          = 0x24;

/// ------------------------------ bmRequestType fields ------------------------------
constexpr uint8_t K_REQUEST_DIRECTION_MASK                = 0x80;
constexpr uint8_t K_REQUEST_DIRECTION_IN                  = 0x80;  // device -> host
constexpr uint8_t K_REQUEST_DIRECTION_OUT                 = 0x00;  // host -> device
constexpr uint8_t K_REQUEST_TYPE_MASK                     = 0x60;
constexpr uint8_t K_REQUEST_TYPE_STANDARD                 = 0x00;
constexpr uint8_t K_REQUEST_TYPE_CLASS                    = 0x20;
constexpr uint8_t K_REQUEST_TYPE_VENDOR                   = 0x40;
constexpr uint8_t K_REQUEST_RECIPIENT_MASK                = 0x1F;
constexpr uint8_t K_REQUEST_RECIPIENT_DEVICE              = 0x00;
constexpr uint8_t K_REQUEST_RECIPIENT_INTERFACE           = 0x01;
constexpr uint8_t K_REQUEST_RECIPIENT_ENDPOINT            = 0x02;

/// ------------------------------ Standard requests ------------------------------
constexpr uint8_t K_REQUEST_GET_STATUS                    = 0x00;
constexpr uint8_t K_REQUEST_CLEAR_FEATURE                 = 0x01;
constexpr uint8_t K_REQUEST_SET_FEATURE                   = 0x03;
constexpr uint8_t K_REQUEST_SET_ADDRESS                   = 0x05;
constexpr uint8_t K_REQUEST_GET_DESCRIPTOR                = 0x06;
constexpr uint8_t K_REQUEST_GET_CONFIGURATION             = 0x08;
constexpr uint8_t K_REQUEST_SET_CONFIGURATION             = 0x09;
constexpr uint8_t K_REQUEST_GET_INTERFACE                 = 0x0A;
constexpr uint8_t K_REQUEST_SET_INTERFACE                 = 0x0B;

/// ------------------------------ Endpoint attributes ------------------------------
constexpr uint8_t K_ENDPOINT_DIRECTION_IN                 = 0x80;
constexpr uint8_t K_ENDPOINT_DIRECTION_OUT                = 0x00;

constexpr uint8_t K_TRANSFER_TYPE_CONTROL                 = 0x00;
constexpr uint8_t K_TRANSFER_TYPE_ISOCHRONOUS             = 0x01;
constexpr uint8_t K_TRANSFER_TYPE_BULK                    = 0x02;
constexpr uint8_t K_TRANSFER_TYPE_INTERRUPT               = 0x03;

/// Full speed control and bulk endpoints are limited to 64-byte packets.
constexpr uint16_t K_FULL_SPEED_MAX_PACKET_SIZE           = 64;

/// Device class triple advertising a composite device built from Interface Association Descriptors.
constexpr uint8_t K_DEVICE_CLASS_MISCELLANEOUS            = 0xEF;
constexpr uint8_t K_DEVICE_SUBCLASS_COMMON                = 0x02;
constexpr uint8_t K_DEVICE_PROTOCOL_IAD                   = 0x01;

/// Language ID for the string descriptor at index 0: English (United States).
constexpr uint16_t K_LANGUAGE_ID_ENGLISH_US               = 0x0409;

/// ------------------------------ On-the-wire structures ------------------------------

/** @brief The 8-byte SETUP packet the host sends at the start of every control transfer. */
struct __attribute__((packed)) SetupPacket {
    uint8_t  request_type;
    uint8_t  request;
    uint16_t value;
    uint16_t index;
    uint16_t length;
};
static_assert(sizeof(SetupPacket) == 8, "SetupPacket must be exactly 8 bytes");

/// ------------------------------ CDC ACM ------------------------------
namespace cdc {

constexpr uint8_t K_CLASS_COMMUNICATIONS           = 0x02;
constexpr uint8_t K_CLASS_DATA                     = 0x0A;
constexpr uint8_t K_SUBCLASS_ACM                   = 0x02;
constexpr uint8_t K_PROTOCOL_AT_COMMANDS           = 0x01;

/// Functional descriptor sub-types (bDescriptorSubtype of a CS_INTERFACE descriptor).
constexpr uint8_t K_FUNCTIONAL_HEADER              = 0x00;
constexpr uint8_t K_FUNCTIONAL_CALL_MANAGEMENT     = 0x01;
constexpr uint8_t K_FUNCTIONAL_ACM                 = 0x02;
constexpr uint8_t K_FUNCTIONAL_UNION               = 0x06;

/// Class-specific requests.
constexpr uint8_t K_REQUEST_SET_LINE_CODING        = 0x20;
constexpr uint8_t K_REQUEST_GET_LINE_CODING        = 0x21;
constexpr uint8_t K_REQUEST_SET_CONTROL_LINE_STATE = 0x22;
constexpr uint8_t K_REQUEST_SEND_BREAK             = 0x23;

/// Bits of wValue in SET_CONTROL_LINE_STATE.
constexpr uint16_t K_CONTROL_LINE_DTR              = 0x0001;
constexpr uint16_t K_CONTROL_LINE_RTS              = 0x0002;

/** @brief Line coding as exchanged with SET_LINE_CODING / GET_LINE_CODING.
 *
 * On a virtual serial port none of this affects the transfer; it is stored so the host can read back what it
 * configured, and so the 1200-baud "touch" that requests a reboot into the bootloader can be recognised.
 */
struct __attribute__((packed)) LineCoding {
    uint32_t baud_rate;
    uint8_t  stop_bits;  // 0 = 1 stop bit, 1 = 1.5, 2 = 2
    uint8_t  parity;     // 0 = none, 1 = odd, 2 = even, 3 = mark, 4 = space
    uint8_t  data_bits;  // 5, 6, 7, 8 or 16
};
static_assert(sizeof(LineCoding) == 7, "LineCoding must be exactly 7 bytes");

/// The baud rate a host sets, then drops DTR, to ask the device to reboot into its bootloader.
constexpr uint32_t K_BOOTLOADER_TOUCH_BAUD_RATE = 1200;

}  // namespace cdc

}  // namespace drivers::usb

#endif  // USBPROTOCOL_H
