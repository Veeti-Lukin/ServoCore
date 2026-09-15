#ifndef USBCDCCHANNELBASE_H
#define USBCDCCHANNELBASE_H

#include <cstdint>
#include <span>

#include "drivers/interfaces/BufferedSerialCommunicationInterface.h"
#include "drivers/usb/UsbDeviceBase.h"
#include "drivers/usb/usb_protocol.h"

namespace drivers::usb {

/**
 * @brief The size-independent part of a CDC ACM virtual serial port.
 *
 * Handles interface ownership, the class control requests (line coding, DTR/RTS, break) and the connection
 * state. The ring buffers and the data path live in the UsbCdcChannel template so that this code is compiled
 * once regardless of buffer sizes.
 */
class UsbCdcChannelBase : public UsbFunctionBase, public interfaces::BufferedSerialCommunicationInterface {
public:
    /// Order of this function's endpoints in Cdc::K_ENDPOINTS.
    static constexpr uint8_t K_ENDPOINT_NOTIFICATION = 0;
    static constexpr uint8_t K_ENDPOINT_DATA_OUT     = 1;
    static constexpr uint8_t K_ENDPOINT_DATA_IN      = 2;

    /**
     * @brief Attach to a device. Called by the UsbDevice facade during construction; the plan provides the numbers.
     * @param device          The device core.
     * @param function_index  Position in the device's function list.
     * @param first_interface Number of the communication interface; the data interface is the next one.
     */
    void bind(UsbDeviceBase& device, uint8_t function_index, uint8_t first_interface);

    /**
     * @brief Whether something on the host side is actually listening.
     *
     * True once the device is configured, the host has asserted DTR (a terminal or the control API opened the
     * port), the bus is not suspended and VBUS is present. Transmit drops data while this is false rather than
     * blocking, because unlike a UART nothing drains the buffer when no one is reading.
     */
    [[nodiscard]] bool isConnected() const;

    [[nodiscard]] bool                   getDtr() const { return dtr_; }
    [[nodiscard]] bool                   getRts() const { return rts_; }
    [[nodiscard]] const cdc::LineCoding& getLineCoding() const { return line_coding_; }

    /// ------------------------------ UsbFunctionBase ------------------------------
    [[nodiscard]] bool ownsInterface(uint8_t interface_number) const override;
    int                onControlIn(const SetupPacket& setup, std::span<uint8_t> response) override;
    bool               onControlOut(const SetupPacket& setup, std::span<const uint8_t> data) override;
    void               onConfigured() override;
    void               onDisconnected() override;

protected:
    /** @brief Forget buffered data and (re)arm reception. Called on configure and on disconnect. */
    virtual void resetLink()          = 0;

    UsbDeviceBase* device_            = nullptr;
    uint8_t        function_index_    = 0;
    uint8_t        control_interface_ = 0;
    uint8_t        data_interface_    = 0;
    EndpointState* out_endpoint_      = nullptr;  // host -> device data
    EndpointState* in_endpoint_       = nullptr;  // device -> host data

    volatile bool configured_         = false;

private:
    cdc::LineCoding line_coding_ = {115200, 0, 0, 8};
    volatile bool   dtr_         = false;
    volatile bool   rts_         = false;
};

}  // namespace drivers::usb

#endif  // USBCDCCHANNELBASE_H
