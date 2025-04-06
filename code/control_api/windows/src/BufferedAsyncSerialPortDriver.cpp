#include "control_api/windows/internal/BufferedAsyncSerialportDriver.h"

#include <format>
#include <stdexcept>

namespace servo_core_control_api::windows::internal {

BufferedAsyncSerialPortDriver::BufferedAsyncSerialPortDriver(const char* serial_port_name) {
    /*The first argument to CreateFile is simply the name of the file you want to open. In this
    case, we want to open a serial port, so we use "COM1", "COM2", etc. The next argument tells Windows
    whether you want to read or write to the serial port. If you don’t need to do one of these, just leave it
    out. Arguments 3 and 4 should pretty much always be 0. The next argument tells us that Windows should
    only open an existing file, and since serial ports already exist, this is what we want. After this comes
    FILE ATTRIBUTE NORMAL, which just tells Windows we don’t want anything fancy here. The final argument
    should also be zero.*/
    serial_port_handle_ =
        CreateFile(serial_port_name, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if (serial_port_handle_ == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            throw std::runtime_error("could not open serial port: The serial port does not exist");
        }
        throw std::runtime_error(std::format("could not open serial port: {}", getLastErrorStr()));
    }

    DCB serial_params       = {0};
    serial_params.DCBlength = sizeof(serial_params);
    if (!GetCommState(serial_port_handle_, &serial_params)) {
        // error getting state
    }
    serial_params.BaudRate = CBR_115200;
    serial_params.ByteSize = 8;
    serial_params.StopBits = ONESTOPBIT;
    serial_params.Parity   = NOPARITY;
    if (!SetCommState(serial_port_handle_, &serial_params)) {
        // error setting serial port state
    }
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
}

void BufferedAsyncSerialPortDriver::transmitBytes(std::span<uint8_t> bytes) {
    for (const uint8_t byte : bytes) {
        transmitByte(byte);
    }
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
    DWORD   read;
    if (!ReadFile(serial_port_handle_, &byte, 1, &read, nullptr)) {
        throw std::runtime_error(std::format("could not read from serial port: {}", getLastErrorStr()));
    }
    return byte;
}

size_t BufferedAsyncSerialPortDriver::readReceivedBytes(std::span<uint8_t> buffer) {
    DWORD bytes_read = 0;
    if (!ReadFile(serial_port_handle_, buffer.data(), 1, &bytes_read, nullptr)) {
        throw std::runtime_error(std::format("could not read from serial port: {}", getLastErrorStr()));
    }

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