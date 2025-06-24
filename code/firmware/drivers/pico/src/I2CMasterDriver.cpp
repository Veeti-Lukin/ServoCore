#include "drivers/I2CMasterDriver.h"

#include "assert/assert.h"

namespace drivers {

constexpr uint8_t K_7BIT_MAX = 127;

I2CMasterDriver::I2CMasterDriver(i2c_inst_t* i2c_block, uint32_t bit_rate)
    : hw_block_(i2c_block), target_bit_rate_(bit_rate) {}

void I2CMasterDriver::init() { output_bit_rate_ = i2c_init(hw_block_, target_bit_rate_); }

void I2CMasterDriver::deInit() { i2c_deinit(hw_block_); }

void I2CMasterDriver::configure(uint32_t bit_rate) {
    target_bit_rate_ = bit_rate;
    output_bit_rate_ = i2c_set_baudrate(hw_block_, bit_rate);
}

I2CMasterDriver::TransmissionResult I2CMasterDriver::transmitByteRaw(uint8_t slave_address, uint8_t data,
                                                                     bool stop_condition_after_the_transmission) {
    ASSERT_WITH_MESSAGE(slave_address <= K_7BIT_MAX, "I2C Slave address does not fit in 7 bits");

    int result = i2c_write_blocking(hw_block_, slave_address, &data, 1, !stop_condition_after_the_transmission);
    return {mapPicoResultToStatus(result), static_cast<size_t>(result)};
}

I2CMasterDriver::TransmissionResult I2CMasterDriver::transmitByteRawWithTimeout(
    uint8_t slave_address, uint8_t data, size_t timeout_us, bool stop_condition_after_the_transmission) {
    ASSERT_WITH_MESSAGE(slave_address <= K_7BIT_MAX, "I2C Slave address does not fit in 7 bits");

    int result =
        i2c_write_timeout_us(hw_block_, slave_address, &data, 1, !stop_condition_after_the_transmission, timeout_us);
    return {mapPicoResultToStatus(result), static_cast<size_t>(result)};
}

I2CMasterDriver::TransmissionResult I2CMasterDriver::transmitBytesRaw(uint8_t slave_address, std::span<uint8_t> bytes,
                                                                      bool stop_condition_after_the_transmission) {
    ASSERT_WITH_MESSAGE(slave_address <= K_7BIT_MAX, "I2C Slave address does not fit in 7 bits");

    int result = i2c_write_blocking(hw_block_, slave_address, bytes.data(), bytes.size_bytes(),
                                    !stop_condition_after_the_transmission);
    return {mapPicoResultToStatus(result), static_cast<size_t>(result)};
}

I2CMasterDriver::TransmissionResult I2CMasterDriver::transmitBytesRawWithTimeout(
    uint8_t slave_address, std::span<uint8_t> bytes, size_t timeout_us, bool stop_condition_after_the_transmission) {
    ASSERT_WITH_MESSAGE(slave_address <= K_7BIT_MAX, "I2C Slave address does not fit in 7 bits");

    int result = i2c_write_timeout_us(hw_block_, slave_address, bytes.data(), bytes.size_bytes(),
                                      !stop_condition_after_the_transmission, timeout_us);
    return {mapPicoResultToStatus(result), static_cast<size_t>(result)};
}

I2CMasterDriver::TransmissionResult I2CMasterDriver::receiveByteRaw(uint8_t slave_address, uint8_t& target_buffer,
                                                                    bool stop_condition_after_the_transmission) {
    ASSERT_WITH_MESSAGE(slave_address <= K_7BIT_MAX, "I2C Slave address does not fit in 7 bits");

    int result = i2c_read_blocking(hw_block_, slave_address, &target_buffer, 1, !stop_condition_after_the_transmission);
    return {mapPicoResultToStatus(result), static_cast<size_t>(result)};
}

I2CMasterDriver::TransmissionResult I2CMasterDriver::receiveByteRawWithTimeout(
    uint8_t slave_address, uint8_t& target_buffer, size_t timeout_us, bool stop_condition_after_the_transmission) {
    ASSERT_WITH_MESSAGE(slave_address <= K_7BIT_MAX, "I2C Slave address does not fit in 7 bits");

    int result = i2c_read_timeout_us(hw_block_, slave_address, &target_buffer, 1,
                                     !stop_condition_after_the_transmission, timeout_us);
    return {mapPicoResultToStatus(result), static_cast<size_t>(result)};
}

I2CMasterDriver::TransmissionResult I2CMasterDriver::receiveBytesRaw(uint8_t            slave_address,
                                                                     std::span<uint8_t> target_buffer,
                                                                     bool stop_condition_after_the_transmission) {
    ASSERT_WITH_MESSAGE(slave_address <= K_7BIT_MAX, "I2C Slave address does not fit in 7 bits");

    int result = i2c_read_blocking(hw_block_, slave_address, target_buffer.data(), target_buffer.size_bytes(),
                                   !stop_condition_after_the_transmission);
    return {mapPicoResultToStatus(result), static_cast<size_t>(result)};
}

I2CMasterDriver::TransmissionResult I2CMasterDriver::receiveBytesRawWithTimeout(
    uint8_t slave_address, std::span<uint8_t> target_buffer, size_t timeout_us,
    bool stop_condition_after_the_transmission) {
    ASSERT_WITH_MESSAGE(slave_address <= K_7BIT_MAX, "I2C Slave address does not fit in 7 bits");

    int result = i2c_read_timeout_us(hw_block_, slave_address, target_buffer.data(), target_buffer.size_bytes(),
                                     !stop_condition_after_the_transmission, timeout_us);
    return {mapPicoResultToStatus(result), static_cast<size_t>(result)};
}

I2CMasterDriver::TransmissionResult I2CMasterDriver::writeRegister(uint8_t            slave_address,
                                                                   std::span<uint8_t> register_address,
                                                                   std::span<uint8_t> data,
                                                                   bool stop_condition_after_the_transmission) {
    // Write slave address and the register address to bus
    // No stop condition to retain the control over the bus
    TransmissionResult register_address_write_result = transmitBytesRaw(slave_address, register_address, false);
    // something went wrong either we got nack or we did not send the whole register address
    if (register_address_write_result.status != Status::ok ||
        register_address_write_result.bytes_operated != register_address.size_bytes()) {
        // return 0 bytes operated, 0 bytes were written to the register
        return {register_address_write_result.status, 0};
    }

    // Write slave address and register data to bus
    // Last transmission was issued with no stop so this one will start with RESTART
    TransmissionResult register_write_result =
        transmitBytesRaw(slave_address, data, stop_condition_after_the_transmission);

    return register_write_result;
}

I2CMasterDriver::TransmissionResult I2CMasterDriver::writeRegister(uint8_t slave_address, uint8_t register_address,
                                                                   std::span<uint8_t> data,
                                                                   bool stop_condition_after_the_transmission) {
    uint8_t register_address_temp[] = {register_address};
    return writeRegister(slave_address, register_address_temp, data, stop_condition_after_the_transmission);
}

I2CMasterDriver::TransmissionResult I2CMasterDriver::writeRegisterWithTimeout(
    uint8_t slave_address, std::span<uint8_t> register_address, std::span<uint8_t> data, size_t timeout_us,
    bool stop_condition_after_the_transmission) {
    // Write slave address and the register address to bus
    // No stop condition to retain the control over the bus
    TransmissionResult register_address_write_result =
        transmitBytesRawWithTimeout(slave_address, register_address, timeout_us / 2, false);
    // something went wrong either we got nack or we did not send the whole register address
    if (register_address_write_result.status != Status::ok ||
        register_address_write_result.bytes_operated != register_address.size_bytes()) {
        // return 0 bytes operated, 0 bytes were read from the register
        return {register_address_write_result.status, 0};
    }

    // Write slave address and register data to bus
    // Last transmission was issued with no stop so this one will start with RESTART
    TransmissionResult register_read_result =
        transmitBytesRawWithTimeout(slave_address, data, timeout_us / 2, stop_condition_after_the_transmission);

    return register_read_result;
}

I2CMasterDriver::TransmissionResult I2CMasterDriver::writeRegisterWithTimeout(
    uint8_t slave_address, uint8_t register_address, std::span<uint8_t> data, size_t timeout_us,
    bool stop_condition_after_the_transmission) {
    uint8_t register_address_temp[] = {register_address};
    return writeRegisterWithTimeout(slave_address, register_address_temp, data, timeout_us,
                                    stop_condition_after_the_transmission);
}

I2CMasterDriver::TransmissionResult I2CMasterDriver::readRegister(uint8_t            slave_address,
                                                                  std::span<uint8_t> register_address,
                                                                  std::span<uint8_t> target_buffer,
                                                                  bool stop_condition_after_the_transmission) {
    // Write slave address and the register address to bus
    // No stop condition to retain the control over the bus
    TransmissionResult register_address_write_result = transmitBytesRaw(slave_address, register_address, false);
    // something went wrong either we got nack or we did not send the whole register address
    if (register_address_write_result.status != Status::ok ||
        register_address_write_result.bytes_operated != register_address.size_bytes()) {
        // return 0 bytes operated, 0 bytes were read from the register
        return {register_address_write_result.status, 0};
    }

    // Write slave address and register data to bus
    // Last transmission was issued with no stop so this one will start with RESTART
    TransmissionResult register_read_result =
        receiveBytesRaw(slave_address, target_buffer, stop_condition_after_the_transmission);

    return register_read_result;
}

I2CMasterDriver::TransmissionResult I2CMasterDriver::readRegister(uint8_t slave_address, uint8_t register_address,
                                                                  std::span<uint8_t> target_buffer,
                                                                  bool stop_condition_after_the_transmission) {
    uint8_t register_address_temp[] = {register_address};
    return readRegister(slave_address, register_address_temp, target_buffer, stop_condition_after_the_transmission);
}

I2CMasterDriver::TransmissionResult I2CMasterDriver::readRegisterWithTimeout(
    uint8_t slave_address, std::span<uint8_t> register_address, std::span<uint8_t> target_buffer, size_t timeout_us,
    bool stop_condition_after_the_transmission) {
    // Write slave address and the register address to bus
    // No stop condition to retain the control over the bus
    TransmissionResult register_address_write_result =
        transmitBytesRawWithTimeout(slave_address, register_address, timeout_us / 2, false);
    // something went wrong either we got nack or we did not send the whole register address
    if (register_address_write_result.status != Status::ok ||
        register_address_write_result.bytes_operated != register_address.size_bytes()) {
        // return 0 bytes operated, 0 bytes were read from the register
        return {register_address_write_result.status, 0};
    }

    // Write slave address and register data to bus
    // Last transmission was issued with no stop so this one will start with RESTART
    TransmissionResult register_read_result =
        receiveBytesRawWithTimeout(slave_address, target_buffer, timeout_us / 2, stop_condition_after_the_transmission);

    return register_read_result;
}

I2CMasterDriver::TransmissionResult I2CMasterDriver::readRegisterWithTimeout(
    uint8_t slave_address, uint8_t register_address, std::span<uint8_t> target_buffer, size_t timeout_us,
    bool stop_condition_after_the_transmission) {
    uint8_t register_address_temp[] = {register_address};
    return readRegisterWithTimeout(slave_address, register_address_temp, target_buffer, timeout_us,
                                   stop_condition_after_the_transmission);
}

I2CMasterDriver::TransmissionResult I2CMasterDriver::burstTransmitBytesRaw(uint8_t            slave_address,
                                                                           std::span<uint8_t> data) {
    int result = i2c_write_burst_blocking(hw_block_, slave_address, data.data(), data.size_bytes());
    return {mapPicoResultToStatus(result), static_cast<size_t>(result)};
}
I2CMasterDriver::TransmissionResult I2CMasterDriver::burstReceiveBytesRaw(uint8_t            slave_address,
                                                                          std::span<uint8_t> target_buffer) {
    int result = i2c_read_burst_blocking(hw_block_, slave_address, target_buffer.data(), target_buffer.size_bytes());
    return {mapPicoResultToStatus(result), static_cast<size_t>(result)};
}

I2CMasterDriver::Status I2CMasterDriver::mapPicoResultToStatus(const int pico_result) {
    switch (pico_result) {
        case PICO_ERROR_TIMEOUT:
            return Status::timeout;
        case PICO_ERROR_GENERIC:
            return Status::nack;
        default:
            return Status::ok;
    }
}

}  // namespace drivers