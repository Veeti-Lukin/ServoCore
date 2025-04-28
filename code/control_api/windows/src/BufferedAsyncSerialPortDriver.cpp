#include "control_api/windows/internal/BufferedAsyncSerialportDriver.h"

#include <format>
#include <stdexcept>
#ifdef SERVO_CORE_CONTROL_API_WINDOWS_COMPORT_DRIVER_DEBUG_PRINTS
#include <iostream>
#endif

namespace servo_core_control_api::windows::internal {

// TODO RENAME BUFFEREDCOMPORTDRIVER since this is not really async wihtout using overlappedio
BufferedAsyncSerialPortDriver::BufferedAsyncSerialPortDriver(const char* serial_port_name) {
    /*The first argument to CreateFile is simply the name of the file you want to open. In this
    case, we want to open a serial port, so we use "COM1", "COM2", etc. The next argument tells Windows
    whether you want to read or write to the serial port. If you don’t need to do one of these, just leave it
    out. Arguments 3 and 4 should pretty much always be 0. The next argument tells us that Windows should
    only open an existing file, and since serial ports already exist, this is what we want. After this comes
    FILE ATTRIBUTE NORMAL, which just tells Windows we don’t want anything fancy here. The final argument
    should also be zero.*/
    serial_port_handle_ =
        CreateFile(serial_port_name, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if (serial_port_handle_ == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            throw std::runtime_error("could not open serial port: The serial port does not exist");
        }
        throw std::runtime_error(std::format("could not open serial port: {}", getLastErrorStr()));
    }

    // Purge RX and TX buffers to clear out garbage that might exist in the windows com port buffers
    PurgeComm(serial_port_handle_, PURGE_RXCLEAR | PURGE_TXCLEAR);

    DCB serial_params       = {0};
    serial_params.DCBlength = sizeof(serial_params);
    if (!GetCommState(serial_port_handle_, &serial_params)) {
        // error getting state
        throw std::runtime_error(std::format("could not get serial parameters: {}", getLastErrorStr()));
    }
    serial_params.BaudRate = CBR_115200;
    serial_params.ByteSize = 8;
    serial_params.StopBits = ONESTOPBIT;
    serial_params.Parity   = NOPARITY;
    if (!SetCommState(serial_port_handle_, &serial_params)) {
        // error setting serial port state
        throw std::runtime_error(std::format("could not set serial parameters: {}", getLastErrorStr()));
    }

    // TODO CALCULATE THESE TIMEOUTS FROM SERIAL_COMMUNICATION_FRAMEWORK TIMEOUTS SO THAT THESE ARE ALWAYS SHORTER?
    COMMTIMEOUTS timeouts                = {};
    timeouts.ReadIntervalTimeout         = 25;  // Time between received bytes (ms)
    timeouts.ReadTotalTimeoutConstant    = 50;  // Total timeout if no bytes received (ms)
    timeouts.ReadTotalTimeoutMultiplier  = 10;  // Additional time per byte expected
    timeouts.WriteTotalTimeoutConstant   = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
}

BufferedAsyncSerialPortDriver::~BufferedAsyncSerialPortDriver() {
    if (serial_port_handle_ != INVALID_HANDLE_VALUE) {
        CloseHandle(serial_port_handle_);
    }
}

void BufferedAsyncSerialPortDriver::transmitByte(uint8_t byte) {
    DWORD written;
    if (!WriteFile(serial_port_handle_, &byte, 1, &written, nullptr)) {
        throw std::runtime_error(std::format("could not write to serial port: {}", getLastErrorStr()));
    }

    if (written != 1) {
        throw std::runtime_error("Serial port writing timed out");
    }

#ifdef SERVO_CORE_CONTROL_API_WINDOWS_COMPORT_DRIVER_DEBUG_PRINTS
    std::cout << "Wrote: " << std::hex << static_cast<int>(byte) << std::endl;
#endif
}

void BufferedAsyncSerialPortDriver::transmitBytes(std::span<uint8_t> bytes) {
    DWORD written;
    if (!WriteFile(serial_port_handle_, bytes.data(), bytes.size(), &written, nullptr)) {
        throw std::runtime_error(std::format("could not write to serial port: {}", getLastErrorStr()));
    }

    if (written != bytes.size()) {
        throw std::runtime_error("Serial port writing timed out");
    }

#ifdef SERVO_CORE_CONTROL_API_WINDOWS_COMPORT_DRIVER_DEBUG_PRINTS
    std::cout << "Wrote: ";
    for (const uint8_t byte : bytes) {
        std::cout << std::hex << static_cast<int>(byte) << std::endl;
    }
    std::cout << std::endl;
#endif
}

size_t BufferedAsyncSerialPortDriver::getReceivedBytesAvailableAmount() {
    COMSTAT comStat;
    DWORD   errors;
    if (ClearCommError(serial_port_handle_, &errors, &comStat)) {
        return comStat.cbInQue;
    }

    return 0;
}

uint8_t BufferedAsyncSerialPortDriver::readReceivedByte() {
    uint8_t byte;
    DWORD   bytes_read = 0;
    if (!ReadFile(serial_port_handle_, &byte, 1, &bytes_read, nullptr)) {
        throw std::runtime_error(std::format("could not read from serial port: {}", getLastErrorStr()));
    }

    if (bytes_read != 1) {
        throw std::runtime_error("Serial port reading timed out");
    }

#ifdef SERVO_CORE_CONTROL_API_WINDOWS_COMPORT_DRIVER_DEBUG_PRINTS
    std::cout << "Read: " << std::hex << static_cast<int>(byte) << std::endl;
#endif

    return byte;
}

size_t BufferedAsyncSerialPortDriver::readReceivedBytes(std::span<uint8_t> buffer) {
    DWORD bytes_read    = 0;
    DWORD bytes_to_read = (buffer.size_bytes() < getReceivedBytesAvailableAmount()) ? buffer.size_bytes()
                                                                                    : getReceivedBytesAvailableAmount();
    if (!ReadFile(serial_port_handle_, buffer.data(), bytes_to_read, &bytes_read, nullptr)) {
        throw std::runtime_error(std::format("could not read from serial port: {}", getLastErrorStr()));
    }

    if (bytes_read != bytes_to_read) {
        throw std::runtime_error("Serial port reading timed out");
    }

#ifdef SERVO_CORE_CONTROL_API_WINDOWS_COMPORT_DRIVER_DEBUG_PRINTS
    std::cout << "Read:";
    for (uint8_t& byte : buffer) {
        std::cout << static_cast<int>(byte);
    }
    std::cout << " " << std::hex << std::endl;
#endif

    return bytes_read;
}

std::string BufferedAsyncSerialPortDriver::getLastErrorStr() {
    constexpr size_t K_ERROR_STRING_BUFF_SIZE              = 1024;
    char             string_buff[K_ERROR_STRING_BUFF_SIZE] = {};
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, GetLastError(),
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), string_buff, K_ERROR_STRING_BUFF_SIZE, nullptr);

    return string_buff;
}

}  // namespace servo_core_control_api::windows::internal