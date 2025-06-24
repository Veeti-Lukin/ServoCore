#ifndef FIRMWARE_DRIVERS_PICO_I2CMASTERDRIVER_H
#define FIRMWARE_DRIVERS_PICO_I2CMASTERDRIVER_H

#include <hardware/i2c.h>

#include <cstdint>
#include <span>

#include "drivers/interfaces/I2CMasterInterface.h"

namespace drivers {

/**
 * @brief Blocking I²C master driver
 *
 * Provides convenient high-level methods for transmitting and receiving
 * data over I²C. Supports raw byte I/O, register-based access,
 * timeout-aware operations, and burst transfers.
 *
 * All methods are blocking until transfer completes, timeouts or an error occurs.
 *  @{
 */
class I2CMasterDriver : public interfaces::I2CMasterInterface {
public:
    /**
     * @brief Construct a new I2C master driver.
     * @param i2c_block Pointer to the I²C hardware instance (i2c0 or i2c1).
     * @param bit_rate Target I²C bit rate in Hz.
     */
    I2CMasterDriver(i2c_inst_t* i2c_block, uint32_t bit_rate);

    /**
     * @brief Initialize the I2C block with the configured target bit rate.
     */
    void init();
    /**
     * @brief Deinitialize the I²C peripheral.
     */
    void deInit();

    /**
     * @brief Reconfigure the I²C bit rate.
     * @param bit_rate New target bit rate in Hz.
     */
    void configure(uint32_t bit_rate) override;

    // ------------------------------------------------------------------------
    // @name  Low-level raw byte I/O
    // @ingroup I2CMasterDriver
    // ------------------------------------------------------------------------

    /**
     * @brief Transmit a single byte to the given slave address.
     * @param slave_address 7-bit I²C slave address.
     * @param data Byte to send.
     * @param stop_condition_after_the_transmission Whether to issue STOP after transmission.
     * @return TransmissionResult containing status and byte count.
     */
    [[nodiscard]] TransmissionResult transmitByteRaw(uint8_t slave_address, uint8_t data,
                                                     bool stop_condition_after_the_transmission = true) override;
    /**
     * @brief Transmit a single byte with timeout.
     * @param slave_address 7-bit I²C slave address.
     * @param data Byte to send.
     * @param timeout_us Timeout in microseconds.
     * @param stop_condition_after_the_transmission Whether to issue STOP after transmission.
     */
    [[nodiscard]] TransmissionResult transmitByteRawWithTimeout(
        uint8_t slave_address, uint8_t data, size_t timeout_us,
        bool stop_condition_after_the_transmission = true) override;
    /**
     * @brief Transmit multiple bytes to the given slave.
     * @param slave_address 7-bit I²C slave address.
     * @param bytes Span of bytes to send.
     * @param stop_condition_after_the_transmission Whether to issue STOP after transmission.
     */
    [[nodiscard]] TransmissionResult transmitBytesRaw(uint8_t slave_address, std::span<uint8_t> bytes,
                                                      bool stop_condition_after_the_transmission = true) override;
    /**
     * @brief Transmit multiple bytes with timeout.
     * @param slave_address 7-bit I²C slave address.
     * @param bytes Span of bytes to send.
     * @param timeout_us Timeout in microseconds.
     * @param stop_condition_after_the_transmission Whether to issue STOP after transmission.
     * @return TransmissionResult containing status and number of bytes transferred.
     */
    [[nodiscard]] TransmissionResult transmitBytesRawWithTimeout(
        uint8_t slave_address, std::span<uint8_t> bytes, size_t timeout_us,
        bool stop_condition_after_the_transmission = true) override;

    /**
     * @brief Receive a single byte from the slave.
     * @param slave_address 7-bit I²C slave address.
     * @param target_buffer Reference where received byte will be stored.
     * @param stop_condition_after_the_transmission Whether to issue STOP after reception.
     * @return TransmissionResult containing status and number of bytes transferred.
     */
    [[nodiscard]] TransmissionResult receiveByteRaw(uint8_t slave_address, uint8_t& target_buffer,
                                                    bool stop_condition_after_the_transmission = true) override;
    /**
     * @brief Receive a single byte with timeout.
     * @param slave_address 7-bit I²C slave address.
     * @param target_buffer Reference where received byte will be stored.
     * @param timeout_us Timeout in microseconds.
     * @param stop_condition_after_the_transmission Whether to issue STOP after reception.
     * @return TransmissionResult containing status and number of bytes transferred.
     */
    [[nodiscard]] TransmissionResult receiveByteRawWithTimeout(
        uint8_t slave_address, uint8_t& target_buffer, size_t timeout_us,
        bool stop_condition_after_the_transmission = true) override;
    /**
     * @brief Receive multiple bytes from the slave.
     * @param slave_address 7-bit I²C slave address.
     * @param target_buffer Span where received bytes will be stored.
     * @param stop_condition_after_the_transmission Whether to issue STOP after reception.
     * @return TransmissionResult containing status and number of bytes transferred.
     */
    [[nodiscard]] TransmissionResult receiveBytesRaw(uint8_t slave_address, std::span<uint8_t> target_buffer,
                                                     bool stop_condition_after_the_transmission = true) override;
    /**
     * @brief Receive multiple bytes with timeout.
     * @param slave_address 7-bit I²C slave address.
     * @param target_buffer Span where received bytes will be stored.
     * @param timeout_us Timeout in microseconds.
     * @param stop_condition_after_the_transmission Whether to issue STOP after reception.
     * @return TransmissionResult containing status and number of bytes transferred.
     */
    [[nodiscard]] TransmissionResult receiveBytesRawWithTimeout(
        uint8_t slave_address, std::span<uint8_t> target_buffer, size_t timeout_us,
        bool stop_condition_after_the_transmission = true) override;
    // ------------------------------------------------------------------------
    // @name Register Level Access
    // @ingroup I2CMasterDriver
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
    [[nodiscard]] TransmissionResult writeRegister(uint8_t slave_address, std::span<uint8_t> register_address,
                                                   std::span<uint8_t> data,
                                                   bool stop_condition_after_the_transmission = true) override;
    /**
     * @brief Write data to a register (single-byte register address).
     * @param slave_address 7-bit I²C slave address.
     * @param register_address Single register address byte.
     * @param data Data bytes to write.
     * @param stop_condition_after_the_transmission Whether to issue STOP after transmission.
     * @return TransmissionResult containing status and number of bytes transferred.
     */
    [[nodiscard]] TransmissionResult writeRegister(uint8_t slave_address, uint8_t register_address,
                                                   std::span<uint8_t> data,
                                                   bool stop_condition_after_the_transmission = true) override;
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
    [[nodiscard]] TransmissionResult writeRegisterWithTimeout(
        uint8_t slave_address, std::span<uint8_t> register_address, std::span<uint8_t> data, size_t timeout_us,
        bool stop_condition_after_the_transmission = true) override;
    /**
     * @brief Write data to a register with timeout (single-byte address).
     * @param slave_address 7-bit I²C slave address.
     * @param register_address Single register address byte.
     * @param data Data bytes to write.
     * @param timeout_us Timeout in microseconds.
     * @param stop_condition_after_the_transmission Whether to issue STOP after transmission.
     * @return TransmissionResult containing status and number of bytes transferred.
     */
    [[nodiscard]] TransmissionResult writeRegisterWithTimeout(
        uint8_t slave_address, uint8_t register_address, std::span<uint8_t> data, size_t timeout_us,
        bool stop_condition_after_the_transmission = true) override;

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
    [[nodiscard]] TransmissionResult readRegister(uint8_t slave_address, std::span<uint8_t> register_address,
                                                  std::span<uint8_t> target_buffer,
                                                  bool stop_condition_after_the_transmission = true) override;
    /**
     * @brief Read data from a register (single-byte address).
     * @param slave_address 7-bit I²C slave address.
     * @param register_address Register address byte.
     * @param target_buffer Span where read bytes will be stored.
     * @param stop_condition_after_the_transmission Whether to issue STOP after read.
     * @return TransmissionResult containing status and number of bytes transferred.
     */
    [[nodiscard]] TransmissionResult readRegister(uint8_t slave_address, uint8_t register_address,
                                                  std::span<uint8_t> target_buffer,
                                                  bool stop_condition_after_the_transmission = true) override;
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
    [[nodiscard]] TransmissionResult readRegisterWithTimeout(
        uint8_t slave_address, std::span<uint8_t> register_address, std::span<uint8_t> target_buffer, size_t timeout_us,
        bool stop_condition_after_the_transmission = true) override;
    /**
     * @brief Read data from a register with timeout (single-byte address).
     * @param slave_address 7-bit I²C slave address.
     * @param register_address Register address byte.
     * @param target_buffer Span where read bytes will be stored.
     * @param timeout_us Timeout in microseconds.
     * @param stop_condition_after_the_transmission Whether to issue STOP after read.
     * @return TransmissionResult containing status and number of bytes transferred.
     */
    [[nodiscard]] TransmissionResult readRegisterWithTimeout(
        uint8_t slave_address, uint8_t register_address, std::span<uint8_t> target_buffer, size_t timeout_us,
        bool stop_condition_after_the_transmission = true) override;

    // ------------------------------------------------------------------------
    // @name Burst Transfers
    // @ingroup I2CMasterDriver
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
    [[nodiscard]] TransmissionResult burstTransmitBytesRaw(uint8_t slave_address, std::span<uint8_t> data) override;
    /**
     * @brief Perform a burst read without STOP or RESTART.
     *
     * Useful when reading large blocks of data continuously.
     *
     * @param slave_address 7-bit I²C slave address.
     * @param target_buffer Span where received data will be stored.
     * @return TransmissionResult containing status and number of bytes transferred.
     */
    [[nodiscard]] TransmissionResult burstReceiveBytesRaw(uint8_t            slave_address,
                                                          std::span<uint8_t> target_buffer) override;

private:
    i2c_inst_t* hw_block_;         ///< Hardware I²C instance
    uint32_t    target_bit_rate_;  ///< Desired bit rate
    uint32_t    output_bit_rate_;  ///< Actual bit rate set by hardware

    static Status mapPicoResultToStatus(const int pico_result);
};

/** @} */  // end of I2CMasterDriver group

}  // namespace drivers

#endif  // FIRMWARE_DRIVERS_PICO_I2CMASTERDRIVER_H
