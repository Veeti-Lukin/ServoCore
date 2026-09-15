#ifndef USBCDCCHANNEL_H
#define USBCDCCHANNEL_H

#include <pico/time.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include "drivers/usb/UsbCdcChannelBase.h"
#include "drivers/usb/UsbDeviceBase.h"
#include "drivers/usb/usb_descriptor_plan.h"
#include "drivers/usb/usb_protocol.h"
#include "utils/RingBuffer.h"

namespace drivers::usb {

template <size_t tx_buffer_size, size_t rx_buffer_size>
class UsbCdcChannel;

/**
 * @brief Compile-time description of one CDC ACM virtual serial port, for use in a UsbDevice type list.
 *
 * Two interfaces (communication + data), three endpoints (interrupt IN notification, bulk OUT, bulk IN), one
 * string (the port name). The runtime object is UsbCdcChannel with the given buffer sizes.
 *
 * @tparam tx_buffer_size Size of the transmit ring buffer.
 * @tparam rx_buffer_size Size of the receive ring buffer.
 */
template <size_t tx_buffer_size, size_t rx_buffer_size>
struct Cdc {
    using Channel                                            = UsbCdcChannel<tx_buffer_size, rx_buffer_size>;

    static constexpr uint8_t  K_NUM_INTERFACES               = 2;
    static constexpr uint8_t  K_NUM_STRINGS                  = 1;
    static constexpr uint16_t K_DESCRIPTOR_SIZE              = 66;

    static constexpr uint16_t K_NOTIFICATION_PACKET_SIZE     = 8;
    static constexpr uint8_t  K_NOTIFICATION_INTERVAL_MS     = 16;

    static constexpr std::array<EndpointSpec, 3> K_ENDPOINTS = {{
        {K_ENDPOINT_DIRECTION_IN, K_TRANSFER_TYPE_INTERRUPT, K_NOTIFICATION_PACKET_SIZE},  // K_ENDPOINT_NOTIFICATION
        {K_ENDPOINT_DIRECTION_OUT, K_TRANSFER_TYPE_BULK, K_FULL_SPEED_MAX_PACKET_SIZE},    // K_ENDPOINT_DATA_OUT
        {K_ENDPOINT_DIRECTION_IN, K_TRANSFER_TYPE_BULK, K_FULL_SPEED_MAX_PACKET_SIZE},     // K_ENDPOINT_DATA_IN
    }};

    /** @brief This function's slice of the configuration descriptor, with the numbers the plan assigned. */
    static constexpr std::array<uint8_t, K_DESCRIPTOR_SIZE> buildDescriptor(FunctionLayout layout) {
        const uint8_t control_interface = layout.first_interface;
        const uint8_t data_interface    = static_cast<uint8_t>(layout.first_interface + 1);
        const uint8_t name_string       = layout.first_string_index;
        const uint8_t notification_ep   = layout.endpoint_addresses[UsbCdcChannelBase::K_ENDPOINT_NOTIFICATION];
        const uint8_t data_out_ep       = layout.endpoint_addresses[UsbCdcChannelBase::K_ENDPOINT_DATA_OUT];
        const uint8_t data_in_ep        = layout.endpoint_addresses[UsbCdcChannelBase::K_ENDPOINT_DATA_IN];

        return {
            // Interface Association Descriptor: groups the two interfaces into one function.
            8,
            K_DESCRIPTOR_TYPE_INTERFACE_ASSOCIATION,
            control_interface,            // bFirstInterface
            K_NUM_INTERFACES,             // bInterfaceCount
            cdc::K_CLASS_COMMUNICATIONS,  // bFunctionClass
            cdc::K_SUBCLASS_ACM,          // bFunctionSubClass
            cdc::K_PROTOCOL_AT_COMMANDS,  // bFunctionProtocol
            name_string,                  // iFunction

            // Communication interface.
            9,
            K_DESCRIPTOR_TYPE_INTERFACE,
            control_interface,            // bInterfaceNumber
            0,                            // bAlternateSetting
            1,                            // bNumEndpoints (notification)
            cdc::K_CLASS_COMMUNICATIONS,  // bInterfaceClass
            cdc::K_SUBCLASS_ACM,          // bInterfaceSubClass
            cdc::K_PROTOCOL_AT_COMMANDS,  // bInterfaceProtocol
            name_string,                  // iInterface

            // Header functional descriptor: CDC specification 1.10.
            5,
            K_DESCRIPTOR_TYPE_CS_INTERFACE,
            cdc::K_FUNCTIONAL_HEADER,
            0x10,
            0x01,
            // Call management functional descriptor: no call management, data over the data interface.
            5,
            K_DESCRIPTOR_TYPE_CS_INTERFACE,
            cdc::K_FUNCTIONAL_CALL_MANAGEMENT,
            0x00,
            data_interface,
            // Abstract control management functional descriptor: line coding and serial state supported.
            4,
            K_DESCRIPTOR_TYPE_CS_INTERFACE,
            cdc::K_FUNCTIONAL_ACM,
            0x02,
            // Union functional descriptor: which data interface belongs to this control interface.
            5,
            K_DESCRIPTOR_TYPE_CS_INTERFACE,
            cdc::K_FUNCTIONAL_UNION,
            control_interface,
            data_interface,

            // Notification endpoint (interrupt IN).
            7,
            K_DESCRIPTOR_TYPE_ENDPOINT,
            notification_ep,            // bEndpointAddress
            K_TRANSFER_TYPE_INTERRUPT,  // bmAttributes
            static_cast<uint8_t>(K_NOTIFICATION_PACKET_SIZE & 0xFF),
            static_cast<uint8_t>(K_NOTIFICATION_PACKET_SIZE >> 8),
            K_NOTIFICATION_INTERVAL_MS,  // bInterval

            // Data interface.
            9,
            K_DESCRIPTOR_TYPE_INTERFACE,
            data_interface,     // bInterfaceNumber
            0,                  // bAlternateSetting
            2,                  // bNumEndpoints
            cdc::K_CLASS_DATA,  // bInterfaceClass
            0x00,               // bInterfaceSubClass
            0x00,               // bInterfaceProtocol
            0,                  // iInterface

            // Data OUT endpoint (bulk).
            7,
            K_DESCRIPTOR_TYPE_ENDPOINT,
            data_out_ep,
            K_TRANSFER_TYPE_BULK,
            static_cast<uint8_t>(K_FULL_SPEED_MAX_PACKET_SIZE & 0xFF),
            static_cast<uint8_t>(K_FULL_SPEED_MAX_PACKET_SIZE >> 8),
            0,  // bInterval (ignored for bulk)

            // Data IN endpoint (bulk).
            7,
            K_DESCRIPTOR_TYPE_ENDPOINT,
            data_in_ep,
            K_TRANSFER_TYPE_BULK,
            static_cast<uint8_t>(K_FULL_SPEED_MAX_PACKET_SIZE & 0xFF),
            static_cast<uint8_t>(K_FULL_SPEED_MAX_PACKET_SIZE >> 8),
            0,
        };
    }
};

//
//
//
//
//
//

/**
 * @brief CDC ACM virtual serial port with buffered asynchronous communication support.
 *
 * Implements BufferedSerialCommunicationInterface so it can replace a UART underneath the serial communication
 * framework or debug_print without either noticing. Reception and transmission are interrupt driven through
 * the device core; the ring buffers decouple them from the main loop.
 *
 * Differences from the UART driver that follow from USB being host-driven:
 *  - Transmit never blocks indefinitely. While the host is not connected (see isConnected()) bytes are dropped
 *    and counted; while it is connected but the TX ring is full, transmit waits at most
 *    DeviceConfig::tx_block_timeout_us before dropping.
 *  - Reception is flow controlled: the data OUT endpoint is only armed while the RX ring has room for a whole
 *    packet, so the host is NAKed rather than data being lost.
 *
 * @tparam tx_buffer_size Size of the TX ring buffer (holds tx_buffer_size - 1 bytes).
 * @tparam rx_buffer_size Size of the RX ring buffer (holds rx_buffer_size - 1 bytes). Must exceed one packet.
 */
template <size_t tx_buffer_size, size_t rx_buffer_size>
class UsbCdcChannel final : public UsbCdcChannelBase {
    static_assert(rx_buffer_size > K_FULL_SPEED_MAX_PACKET_SIZE + 1, "RX ring must hold at least one full packet");
    static_assert(tx_buffer_size > 1, "TX ring must hold at least one byte");

public:
    UsbCdcChannel() = default;

    /// ------------------------------ BufferedSerialCommunicationInterface ------------------------------
    void    transmitByte(uint8_t byte) override;
    void    transmitBytes(std::span<uint8_t> bytes) override;
    size_t  getReceivedBytesAvailableAmount() override;
    uint8_t readReceivedByte() override;
    size_t  readReceivedBytes(std::span<uint8_t> bytes) override;

    /**
     * @brief Wait until everything queued has been handed to the host, or until the host stops taking data.
     *
     * Bounded by tx_block_timeout_us per packet; returns early if the host disconnects.
     */
    void flushTx();

    /// ------------------------------ UsbFunctionBase ------------------------------
    void onEndpointComplete(EndpointState& endpoint, uint16_t length) override;

protected:
    void resetLink() override;

private:
    /** @brief Queue one byte, applying the drop / bounded-wait policy. Returns false if it was dropped. */
    bool enqueueTx(uint8_t byte);
    /** @brief Hand the next packet from the TX ring to the controller. Call with the USB interrupt masked. */
    void pumpTx();
    /** @brief Arm the data OUT endpoint if the RX ring has room for a full packet. Call with the interrupt masked. */
    void                 armRxIfRoom();
    [[nodiscard]] size_t rxFreeSpace() const;

    volatile utils::RingBuffer<tx_buffer_size> tx_ring_buffer_;
    volatile utils::RingBuffer<rx_buffer_size> rx_ring_buffer_;
    volatile bool                              rx_armed_ = false;
};

/// ------------------------ DEFINITIONS --------------------------------------

template <size_t tx_buffer_size, size_t rx_buffer_size>
bool UsbCdcChannel<tx_buffer_size, rx_buffer_size>::enqueueTx(uint8_t byte) {
    if (!isConnected()) {
        device_->countTxDrop();
        return false;
    }
    {
        UsbDeviceBase::InterruptGuard guard;
        if (tx_ring_buffer_.push(byte)) {
            return true;
        }
        pumpTx();  // ring is full: make sure the endpoint is draining it before waiting
    }

    // Give the host a bounded amount of time to take a packet, letting the interrupt run in between attempts.
    const uint32_t started_at = time_us_32();
    while (isConnected() && (time_us_32() - started_at) < device_->getTxBlockTimeoutUs()) {
        UsbDeviceBase::InterruptGuard guard;
        if (tx_ring_buffer_.push(byte)) {
            return true;
        }
    }
    device_->countTxDrop();
    return false;
}

template <size_t tx_buffer_size, size_t rx_buffer_size>
void UsbCdcChannel<tx_buffer_size, rx_buffer_size>::transmitByte(uint8_t byte) {
    if (enqueueTx(byte)) {
        UsbDeviceBase::InterruptGuard guard;
        pumpTx();
    }
}

template <size_t tx_buffer_size, size_t rx_buffer_size>
void UsbCdcChannel<tx_buffer_size, rx_buffer_size>::transmitBytes(std::span<uint8_t> bytes) {
    // Queue everything first so the data leaves in full packets rather than a one-byte packet followed by the rest.
    for (size_t i = 0; i < bytes.size(); i++) {
        if (!enqueueTx(bytes[i])) {
            // Not connected, or the host stopped taking data: the rest is dropped and counted as well.
            for (size_t remaining = i + 1; remaining < bytes.size(); remaining++) {
                device_->countTxDrop();
            }
            break;
        }
    }
    UsbDeviceBase::InterruptGuard guard;
    pumpTx();
}

template <size_t tx_buffer_size, size_t rx_buffer_size>
void UsbCdcChannel<tx_buffer_size, rx_buffer_size>::flushTx() {
    const uint32_t started_at = time_us_32();
    while (isConnected() && (time_us_32() - started_at) < device_->getTxBlockTimeoutUs()) {
        UsbDeviceBase::InterruptGuard guard;
        if (tx_ring_buffer_.isEmpty() && !in_endpoint_->busy) {
            return;
        }
    }
}

template <size_t tx_buffer_size, size_t rx_buffer_size>
size_t UsbCdcChannel<tx_buffer_size, rx_buffer_size>::getReceivedBytesAvailableAmount() {
    UsbDeviceBase::InterruptGuard guard;
    return rx_ring_buffer_.bytesAvailable();
}

template <size_t tx_buffer_size, size_t rx_buffer_size>
uint8_t UsbCdcChannel<tx_buffer_size, rx_buffer_size>::readReceivedByte() {
    UsbDeviceBase::InterruptGuard guard;
    const uint8_t                 byte = rx_ring_buffer_.pop();
    armRxIfRoom();
    return byte;
}

template <size_t tx_buffer_size, size_t rx_buffer_size>
size_t UsbCdcChannel<tx_buffer_size, rx_buffer_size>::readReceivedBytes(std::span<uint8_t> bytes) {
    UsbDeviceBase::InterruptGuard guard;
    size_t                        count = 0;
    while (count < bytes.size() && !rx_ring_buffer_.isEmpty()) {
        bytes[count++] = rx_ring_buffer_.pop();
    }
    armRxIfRoom();
    return count;
}

template <size_t tx_buffer_size, size_t rx_buffer_size>
void UsbCdcChannel<tx_buffer_size, rx_buffer_size>::onEndpointComplete(EndpointState& endpoint, uint16_t length) {
    if (&endpoint == in_endpoint_) {
        // The host collected a packet; send the next one if there is anything queued.
        pumpTx();
    } else if (&endpoint == out_endpoint_) {
        rx_armed_ = false;
        for (uint16_t i = 0; i < length; i++) {
            if (!rx_ring_buffer_.push(endpoint.buffer[i])) {
                device_->countRxOverrun();  // cannot happen while armRxIfRoom() is honoured
                break;
            }
        }
        armRxIfRoom();
    }
}

template <size_t tx_buffer_size, size_t rx_buffer_size>
void UsbCdcChannel<tx_buffer_size, rx_buffer_size>::resetLink() {
    UsbDeviceBase::InterruptGuard guard;
    // Drain the rings; RingBuffer has no clear().
    while (!tx_ring_buffer_.isEmpty()) tx_ring_buffer_.pop();
    while (!rx_ring_buffer_.isEmpty()) rx_ring_buffer_.pop();
    rx_armed_ = false;
    if (configured_) {
        armRxIfRoom();
    }
}

template <size_t tx_buffer_size, size_t rx_buffer_size>
void UsbCdcChannel<tx_buffer_size, rx_buffer_size>::pumpTx() {
    if (!configured_ || in_endpoint_->busy || tx_ring_buffer_.isEmpty()) {
        return;
    }
    std::array<uint8_t, K_FULL_SPEED_MAX_PACKET_SIZE> packet{};
    size_t                                            count = 0;
    while (count < packet.size() && !tx_ring_buffer_.isEmpty()) {
        packet[count++] = tx_ring_buffer_.pop();
    }
    device_->startIn(*in_endpoint_, std::span<const uint8_t>(packet.data(), count));
}

template <size_t tx_buffer_size, size_t rx_buffer_size>
void UsbCdcChannel<tx_buffer_size, rx_buffer_size>::armRxIfRoom() {
    if (!configured_ || rx_armed_) {
        return;
    }
    if (rxFreeSpace() >= K_FULL_SPEED_MAX_PACKET_SIZE) {
        rx_armed_ = true;
        device_->armOut(*out_endpoint_);
    }
}

template <size_t tx_buffer_size, size_t rx_buffer_size>
size_t UsbCdcChannel<tx_buffer_size, rx_buffer_size>::rxFreeSpace() const {
    // RingBuffer<size> holds at most size - 1 elements.
    return (rx_buffer_size - 1) - rx_ring_buffer_.bytesAvailable();
}

}  // namespace drivers::usb

#endif  // USBCDCCHANNEL_H
