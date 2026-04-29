#ifndef CONTROL_API_WINDOWS_BUFFEREDASYNCSERIALPORTDRIVER_H
#define CONTROL_API_WINDOWS_BUFFEREDASYNCSERIALPORTDRIVER_H

#include <windows.h>

#include <format>
#include <string>

#include "drivers/interfaces/BufferedSerialCommunicationInterface.h"
#include "utils/RingBuffer.h"

namespace servo_core_control_api::windows::internal {

class BufferedAsyncSerialPortDriver : public drivers::interfaces::BufferedSerialCommunicationInterface {
public:
    explicit BufferedAsyncSerialPortDriver(const char* serial_port_name);
    ~        BufferedAsyncSerialPortDriver() override;

    void transmitByte(uint8_t byte) override;

    void transmitBytes(std::span<uint8_t> bytes) override;

    size_t getReceivedBytesAvailableAmount() override;

    uint8_t readReceivedByte() override;

    size_t readReceivedBytes(std::span<uint8_t> buffer) override;

private:
    std::string getLastErrorStr();

    /**
     * @brief Normalize a COM port name to the Win32 device-namespace form `\\.\COMn`.
     *
     * Prepends the `\\.\` prefix if the input does not already start with it. If the input
     * is already prefixed, it is returned unchanged.
     *
     * @note Why this is needed: CreateFile("COM1") ... CreateFile("COM9") work via the
     *       legacy DOS device namespace, but CreateFile("COM10") and any higher-numbered
     *       port silently fail (returns INVALID_HANDLE_VALUE) without the `\\.\` prefix.
     *       The `\\.\COMn` form works for all port numbers, so we always normalize before
     *       opening to avoid this Windows quirk.
     *
     * @param str  COM port name, with or without the `\\.\` prefix (e.g. "COM3" or "\\.\COM10").
     * @return     The same name guaranteed to be in `\\.\COMn` form.
     */
    static std::string makeValidComPortPath(std::string_view str);

    HANDLE serial_port_handle_;
};

}  // namespace servo_core_control_api::windows::internal

#endif  // CONTROL_API_WINDOWS_BUFFEREDASYNCSERIALPORTDRIVER_H
