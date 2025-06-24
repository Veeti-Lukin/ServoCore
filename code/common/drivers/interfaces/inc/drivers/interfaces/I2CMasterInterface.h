#ifndef COMMON_DRIVERS_INTERFACES_I2CMASTERINTERFACE_H
#define COMMON_DRIVERS_INTERFACES_I2CMASTERINTERFACE_H

#include <cstddef>
#include <span>

namespace drivers::interfaces {

/**
 * @brief Abstract interface for I²C master communication.
 *
 * Provides a hardware-agnostic API for blocking I²C transactions, including:
 * - Raw byte-level transmit and receive
 * - Register-level access (with single- or multi-byte register addresses)
 * - Optional timeout variants for robustness
 * - Burst transfers without issuing STOP/RESTART for efficient block operations
 *
 * This interface allows different hardware-specific I²C master drivers
 * (e.g. RP2040 SDK, STM32 HAL, etc.) to implement a common API, enabling
 * portability and testability of higher-level drivers (such as sensors
 * and actuators).
 *
 * All operations are blocking and return a TransmissionResult that contains
 * the operation status and number of bytes successfully operated on.
 * @{
 */
class I2CMasterInterface {
public:
    virtual ~I2CMasterInterface() = default;
    /**
     * @brief Status codes returned from I²C operations.
     */
    enum class Status {
        ok,
        nack,  ///< Address not acknowledged, device not present, or some data (register address?) not acknowledged
        timeout,

        unknown,
    };
    /**
     * @brief Result of an I²C transmission/reception.
     */
    struct TransmissionResult {
        Status status         = Status::unknown;
        size_t bytes_operated = 0;  ///< Only reliable if status is ok
    };

    /**
     * @brief Reconfigure the I²C bit rate.
     * @param bit_rate New target bit rate in Hz.
     */
    virtual void configure(uint32_t bit_rate)                                                                   = 0;

    // ------------------------------------------------------------------------
    // @name  Low-level raw byte I/O
    // @ingroup I2CMasterInterface
    // ------------------------------------------------------------------------

    /**
     * @brief Transmit a single byte to the given slave address.
     * @param slave_address 7-bit I²C slave address.
     * @param data Byte to send.
     * @param stop_condition_after_the_transmission Whether to issue STOP after transmission.
     * @return TransmissionResult containing status and byte count.
     */
    [[nodiscard]] virtual TransmissionResult transmitByteRaw(uint8_t slave_address, uint8_t data,
                                                             bool stop_condition_after_the_transmission = true) = 0;
    /**
     * @brief Transmit a single byte with timeout.
     * @param slave_address 7-bit I²C slave address.
     * @param data Byte to send.
     * @param timeout_us Timeout in microseconds.
     * @param stop_condition_after_the_transmission Whether to issue STOP after transmission.
     */
    [[nodiscard]] virtual TransmissionResult transmitByteRawWithTimeout(
        uint8_t slave_address, uint8_t data, size_t timeout_us, bool stop_condition_after_the_transmission = true) = 0;
    /**
     * @brief Transmit multiple bytes to the given slave.
     * @param slave_address 7-bit I²C slave address.
     * @param bytes Span of bytes to send.
     * @param stop_condition_after_the_transmission Whether to issue STOP after transmission.
     */
    [[nodiscard]] virtual TransmissionResult transmitBytesRaw(uint8_t slave_address, std::span<uint8_t> bytes,
                                                              bool stop_condition_after_the_transmission = true)   = 0;
    /**
     * @brief Transmit multiple bytes with timeout.
     * @param slave_address 7-bit I²C slave address.
     * @param bytes Span of bytes to send.
     * @param timeout_us Timeout in microseconds.
     * @param stop_condition_after_the_transmission Whether to issue STOP after transmission.
     * @return TransmissionResult containing status and number of bytes transferred.
     */
    [[nodiscard]] virtual TransmissionResult transmitBytesRawWithTimeout(
        uint8_t slave_address, std::span<uint8_t> bytes, size_t timeout_us,
        bool stop_condition_after_the_transmission = true)                                                     = 0;

    /**
     * @brief Receive a single byte from the slave.
     * @param slave_address 7-bit I²C slave address.
     * @param target_buffer Reference where received byte will be stored.
     * @param stop_condition_after_the_transmission Whether to issue STOP after reception.
     * @return TransmissionResult containing status and number of bytes transferred.
     */
    [[nodiscard]] virtual TransmissionResult receiveByteRaw(uint8_t slave_address, uint8_t& target_buffer,
                                                            bool stop_condition_after_the_transmission = true) = 0;
    /**
     * @brief Receive a single byte with timeout.
     * @param slave_address 7-bit I²C slave address.
     * @param target_buffer Reference where received byte will be stored.
     * @param timeout_us Timeout in microseconds.
     * @param stop_condition_after_the_transmission Whether to issue STOP after reception.
     * @return TransmissionResult containing status and number of bytes transferred.
     */
    [[nodiscard]] virtual TransmissionResult receiveByteRawWithTimeout(
        uint8_t slave_address, uint8_t& target_buffer, size_t timeout_us,
        bool stop_condition_after_the_transmission = true)                                                      = 0;
    /**
     * @brief Receive multiple bytes from the slave.
     * @param slave_address 7-bit I²C slave address.
     * @param target_buffer Span where received bytes will be stored.
     * @param stop_condition_after_the_transmission Whether to issue STOP after reception.
     * @return TransmissionResult containing status and number of bytes transferred.
     */
    [[nodiscard]] virtual TransmissionResult receiveBytesRaw(uint8_t slave_address, std::span<uint8_t> target_buffer,
                                                             bool stop_condition_after_the_transmission = true) = 0;
    /**
     * @brief Receive multiple bytes with timeout.
     * @param slave_address 7-bit I²C slave address.
     * @param target_buffer Span where received bytes will be stored.
     * @param timeout_us Timeout in microseconds.
     * @param stop_condition_after_the_transmission Whether to issue STOP after reception.
     * @return TransmissionResult containing status and number of bytes transferred.
     */
    [[nodiscard]] virtual TransmissionResult receiveBytesRawWithTimeout(
        uint8_t slave_address, std::span<uint8_t> target_buffer, size_t timeout_us,
        bool stop_condition_after_the_transmission = true)                                                    = 0;
    // ------------------------------------------------------------------------
    // @name Register Level Access
    // @ingroup I2CMasterInterface
    // ------------------------------------------------------------------------

    /**
     * @brief Write data to a register (multibyte register address).
     *
     *        Useful for multibyte register addresses because you can define endianess for register address
     *
     * @param slave_address 7-bit I²C slave address.
     * @param register_address Span containing register address bytes (e.g., 1 or 2 bytes).
     * @param data Data bytes to write.
     * @param stop_condition_after_the_transmission Whether to issue STOP after transmission.
     * @return TransmissionResult containing status and number of bytes transferred.
     */
    [[nodiscard]] virtual TransmissionResult writeRegister(uint8_t slave_address, std::span<uint8_t> register_address,
                                                           std::span<uint8_t> data,
                                                           bool stop_condition_after_the_transmission = true) = 0;
    /**
     * @brief Write data to a register (single-byte register address).
     * @param slave_address 7-bit I²C slave address.
     * @param register_address Single register address byte.
     * @param data Data bytes to write.
     * @param stop_condition_after_the_transmission Whether to issue STOP after transmission.
     * @return TransmissionResult containing status and number of bytes transferred.
     */
    [[nodiscard]] virtual TransmissionResult writeRegister(uint8_t slave_address, uint8_t register_address,
                                                           std::span<uint8_t> data,
                                                           bool stop_condition_after_the_transmission = true) = 0;
    /**
     * @brief Write data to a register with timeout (multi-byte address).
     *
     *         Useful for multibyte register addresses because you can define endianess for register address
     *
     * @param slave_address 7-bit I²C slave address.
     * @param register_address Span containing register address bytes.
     * @param data Data bytes to write.
     * @param timeout_us Timeout in microseconds.
     * @param stop_condition_after_the_transmission Whether to issue STOP after transmission.
     * @return TransmissionResult containing status and number of bytes transferred.
     */
    [[nodiscard]] virtual TransmissionResult writeRegisterWithTimeout(
        uint8_t slave_address, std::span<uint8_t> register_address, std::span<uint8_t> data, size_t timeout_us,
        bool stop_condition_after_the_transmission = true) = 0;
    /**
     * @brief Write data to a register with timeout (single-byte address).
     * @param slave_address 7-bit I²C slave address.
     * @param register_address Single register address byte.
     * @param data Data bytes to write.
     * @param timeout_us Timeout in microseconds.
     * @param stop_condition_after_the_transmission Whether to issue STOP after transmission.
     * @return TransmissionResult containing status and number of bytes transferred.
     */
    [[nodiscard]] virtual TransmissionResult writeRegisterWithTimeout(
        uint8_t slave_address, uint8_t register_address, std::span<uint8_t> data, size_t timeout_us,
        bool stop_condition_after_the_transmission = true)                                                   = 0;

    /**
     * @brief Read data from a register (multi-byte register address).
     *
     *        Useful for multibyte register addresses because you can define endianess for register address
     *
     * @param slave_address 7-bit I²C slave address.
     * @param register_address Span containing register address bytes.
     * @param target_buffer Span where read bytes will be stored.
     * @param stop_condition_after_the_transmission Whether to issue STOP after read.
     * @return TransmissionResult containing status and number of bytes transferred.
     */
    [[nodiscard]] virtual TransmissionResult readRegister(uint8_t slave_address, std::span<uint8_t> register_address,
                                                          std::span<uint8_t> target_buffer,
                                                          bool stop_condition_after_the_transmission = true) = 0;
    /**
     * @brief Read data from a register (single-byte address).
     * @param slave_address 7-bit I²C slave address.
     * @param register_address Register address byte.
     * @param target_buffer Span where read bytes will be stored.
     * @param stop_condition_after_the_transmission Whether to issue STOP after read.
     * @return TransmissionResult containing status and number of bytes transferred.
     */
    [[nodiscard]] virtual TransmissionResult readRegister(uint8_t slave_address, uint8_t register_address,
                                                          std::span<uint8_t> target_buffer,
                                                          bool stop_condition_after_the_transmission = true) = 0;
    /**
     * @brief Read data from a register with timeout (multi-byte address).
     *
     *        Useful for multibyte register addresses because you can define endianess for register address
     * @param slave_address 7-bit I²C slave address.
     * @param register_address Span containing register address bytes.
     * @param target_buffer Span where read bytes will be stored.
     * @param timeout_us Timeout in microseconds.
     * @param stop_condition_after_the_transmission Whether to issue STOP after read.
     * @return TransmissionResult containing status and number of bytes transferred.
     */
    [[nodiscard]] virtual TransmissionResult readRegisterWithTimeout(
        uint8_t slave_address, std::span<uint8_t> register_address, std::span<uint8_t> target_buffer, size_t timeout_us,
        bool stop_condition_after_the_transmission = true) = 0;
    /**
     * @brief Read data from a register with timeout (single-byte address).
     * @param slave_address 7-bit I²C slave address.
     * @param register_address Register address byte.
     * @param target_buffer Span where read bytes will be stored.
     * @param timeout_us Timeout in microseconds.
     * @param stop_condition_after_the_transmission Whether to issue STOP after read.
     * @return TransmissionResult containing status and number of bytes transferred.
     */
    [[nodiscard]] virtual TransmissionResult readRegisterWithTimeout(
        uint8_t slave_address, uint8_t register_address, std::span<uint8_t> target_buffer, size_t timeout_us,
        bool stop_condition_after_the_transmission = true)                                                         = 0;

    // ------------------------------------------------------------------------
    // @name Burst Transfers
    // @ingroup I2CMasterInterface
    // ------------------------------------------------------------------------
    /**
     * @brief Perform a burst write without issuing STOP or RESTART.
     *
     * Useful for sending consecutive chunks of data efficiently without
     * re-sending the slave address or stop bits.
     *
     * @param slave_address 7-bit I²C slave address.
     * @param data Span of data bytes to send.
     * @return TransmissionResult containing status and number of bytes transferred.
     */
    [[nodiscard]] virtual TransmissionResult burstTransmitBytesRaw(uint8_t slave_address, std::span<uint8_t> data) = 0;
    /**
     * @brief Perform a burst read without STOP or RESTART.
     *
     * Useful when reading large blocks of data continuously.
     *
     * @param slave_address 7-bit I²C slave address.
     * @param target_buffer Span where received data will be stored.
     * @return TransmissionResult containing status and number of bytes transferred.
     */
    [[nodiscard]] virtual TransmissionResult burstReceiveBytesRaw(uint8_t            slave_address,
                                                                  std::span<uint8_t> target_buffer)                = 0;
};

/** @} */  // end of I2CMasterInterface group

}  // namespace drivers::interfaces

#endif  //  COMMON_DRIVERS_INTERFACES_I2CMASTERINTERFACE_H
