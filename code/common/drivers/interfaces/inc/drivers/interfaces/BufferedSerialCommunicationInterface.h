#ifndef SERIALBUFFEREDCOMMUNICATIONINTERFACE_H
#define SERIALBUFFEREDCOMMUNICATIONINTERFACE_H

#include <cstdint>
#include <span>

namespace drivers::interfaces {

/** @brief Interface class for Serial Buffered Communication
 * This inteface defines the basic functionality for transmitting and receiving bytes
 * over a serial connection with buffered communication support.
 */
class BufferedSerialCommunicationInterface {
public:
    virtual ~BufferedSerialCommunicationInterface()            = default;

    /**
     * @brief Transmit a single byte through the communication interface.
     * @param byte The byte to be transmitted.
     */
    virtual void transmitByte(uint8_t byte)                    = 0;
    /**
     * @brief Transmit multiple bytes through the communication interface.
     * @param bytes A span of bytes to be transmitted.
     */
    virtual void transmitBytes(std::span<uint8_t> bytes)       = 0;

    /**
     * @brief Get the number of available bytes in the rx buffer.
     * This function checks how many bytes are currently available to be read from the buffer.
     * @return The number of available received bytes.
     */
    virtual size_t getReceivedBytesAvailableAmount()           = 0;
    /**
     * @brief Read a single byte from the rx buffer.
     * @return The byte read from the rx buffer.
     */
    virtual uint8_t readReceivedByte()                         = 0;
    /**
     * @brief Read multiple bytes from the rx buffer.
     * @param bytes A span of bytes where the received data will be stored.
     * @return The number of bytes successfully read from the rx buffer.
     */
    virtual size_t readReceivedBytes(std::span<uint8_t> bytes) = 0;
};

}  // namespace drivers::interfaces

#endif  // SERIALBUFFEREDCOMMUNICATIONINTERFACE_H
