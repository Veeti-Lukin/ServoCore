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

    HANDLE serial_port_handle_;
};

}  // namespace servo_core_control_api::windows::internal

#endif  // CONTROL_API_WINDOWS_BUFFEREDASYNCSERIALPORTDRIVER_H
