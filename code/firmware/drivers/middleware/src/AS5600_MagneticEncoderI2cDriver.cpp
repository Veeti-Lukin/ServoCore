#include "drivers/middleware/AS5600_MagneticEncoderI2cDriver.h"

#include <numbers>
#include <span>

#include "assert/assert.h"

namespace drivers::middleware {

using TransmissionResult = interfaces::I2CMasterInterface::TransmissionResult;

AS5600_MagneticEncoderI2cDriver::AS5600_MagneticEncoderI2cDriver(interfaces::I2CMasterInterface* i2c_driver)
    : i2c_driver_(i2c_driver) {}

void AS5600_MagneticEncoderI2cDriver::init() { i2c_driver_->configure(K_I2C_BITRATE); }

bool AS5600_MagneticEncoderI2cDriver::isConnected() {
    // Perform a dummy read to some register
    [[maybe_unused]] uint8_t test   = 0;
    TransmissionResult       result = i2c_driver_->readRegisterWithTimeout(
        K_I2C_ADDRESS, static_cast<uint8_t>(RegisterAddresses::raw_angle_h), {&test, 1}, K_I2C_TIMEOUT_US, true);
    // If the address is acknowledged then the device is present on the i2c bus
    return result.status == interfaces::I2CMasterInterface::Status::ok;
}

AS5600_MagneticEncoderI2cDriver::Status AS5600_MagneticEncoderI2cDriver::readStatus() {
    uint8_t            status_value = 0;
    TransmissionResult result       = i2c_driver_->readRegister(
        K_I2C_ADDRESS, static_cast<uint8_t>(RegisterAddresses::status), {&status_value, 1}, true);
    // TODO what to do if i2c communication fails? For now only assert
    ASSERT(result.status == interfaces::I2CMasterInterface::Status::ok && result.bytes_operated == 1);

    Status status;
    status.magnet_detected = status_value & static_cast<uint8_t>(StatusRegisterBits::magnet_detected);
    // Determine magnet strength
    if (status_value & static_cast<uint8_t>(StatusRegisterBits::magnet_too_strong)) {
        status.magnet_strength = Status::MagnetStrength::too_strong;
    } else if (status_value & static_cast<uint8_t>(StatusRegisterBits::magnet_too_weak)) {
        status.magnet_strength = Status::MagnetStrength::too_weak;
    }
    // If neither of the bits are set the magnet strength should be okay
    else {
        status.magnet_strength = Status::MagnetStrength::ok;
    }

    return status;
}

bool AS5600_MagneticEncoderI2cDriver::burn(BurnCommand command) {
    uint8_t            cmd    = static_cast<uint8_t>(command);
    TransmissionResult result = i2c_driver_->writeRegister(K_I2C_ADDRESS, static_cast<uint8_t>(RegisterAddresses::burn),
                                                           {&cmd, 1}, K_I2C_TIMEOUT_US);

    return result.status == interfaces::I2CMasterInterface::Status::ok && result.bytes_operated == 1;
}

bool AS5600_MagneticEncoderI2cDriver::configurePowerMode(PowerMode mode) {
    uint16_t conf_register_value;
    if (!readConfRegister(conf_register_value)) return false;

    // Clear PM bits (bit0, bit1)
    conf_register_value &=
        ~(static_cast<uint16_t>(ConfRegisterBits::pm1) | static_cast<uint16_t>(ConfRegisterBits::pm2));

    // Set bits individually depending on mode
    if (static_cast<uint8_t>(mode) & 0b01) {
        conf_register_value |= static_cast<uint16_t>(ConfRegisterBits::pm1);
    }
    if (static_cast<uint8_t>(mode) & 0b10) {
        conf_register_value |= static_cast<uint16_t>(ConfRegisterBits::pm2);
    }

    return writeConfRegister(conf_register_value);
}

bool AS5600_MagneticEncoderI2cDriver::configureHysteresis(HysteresisMode mode) {
    uint16_t conf_register_value;
    if (!readConfRegister(conf_register_value)) return false;

    // Clear hysteresis bits (bit2, bit3)
    conf_register_value &=
        ~(static_cast<uint16_t>(ConfRegisterBits::hyst1) | static_cast<uint16_t>(ConfRegisterBits::hyst2));

    // Set new hysteresis value explicitly using the masks
    if (static_cast<uint8_t>(mode) & 0b01) {
        conf_register_value |= static_cast<uint16_t>(ConfRegisterBits::hyst1);
    }
    if (static_cast<uint8_t>(mode) & 0b10) {
        conf_register_value |= static_cast<uint16_t>(ConfRegisterBits::hyst2);
    }

    return writeConfRegister(conf_register_value);
}

bool AS5600_MagneticEncoderI2cDriver::enableAutoLowPowerMode(bool enable) {
    uint16_t conf_register_value;
    if (!readConfRegister(conf_register_value)) return false;

    if (enable) {
        conf_register_value |= static_cast<uint16_t>(ConfRegisterBits::wd);
    } else {
        conf_register_value &= ~static_cast<uint16_t>(ConfRegisterBits::wd);
    }

    return writeConfRegister(conf_register_value);
}

bool AS5600_MagneticEncoderI2cDriver::configureNoiseReductionFilter(SlowFilterMode      slow_filter_mode,
                                                                    FastFilterThreshold fast_filter_threshold) {
    uint16_t conf_register_value;
    if (!readConfRegister(conf_register_value)) return false;

    // ---- Slow filter (bits 8–9) ----
    conf_register_value &=
        ~(static_cast<uint16_t>(ConfRegisterBits::sf1) | static_cast<uint16_t>(ConfRegisterBits::sf2));

    if (static_cast<uint8_t>(slow_filter_mode) & 0b01) {
        conf_register_value |= static_cast<uint16_t>(ConfRegisterBits::sf1);
    }
    if (static_cast<uint8_t>(slow_filter_mode) & 0b10) {
        conf_register_value |= static_cast<uint16_t>(ConfRegisterBits::sf2);
    }

    // ---- Fast filter threshold (bits 10–12) ----
    conf_register_value &=
        ~(static_cast<uint16_t>(ConfRegisterBits::fth1) | static_cast<uint16_t>(ConfRegisterBits::fth2) |
          static_cast<uint16_t>(ConfRegisterBits::fth3));

    if (static_cast<uint8_t>(fast_filter_threshold) & 0b001) {
        conf_register_value |= static_cast<uint16_t>(ConfRegisterBits::fth1);
    }
    if (static_cast<uint8_t>(fast_filter_threshold) & 0b010) {
        conf_register_value |= static_cast<uint16_t>(ConfRegisterBits::fth2);
    }
    if (static_cast<uint8_t>(fast_filter_threshold) & 0b100) {
        conf_register_value |= static_cast<uint16_t>(ConfRegisterBits::fth3);
    }

    return writeConfRegister(conf_register_value);
}

uint64_t AS5600_MagneticEncoderI2cDriver::getMaxAngleCounts() { return K_MAX_ABSOLUTE_ANGLE_READING; }

uint64_t AS5600_MagneticEncoderI2cDriver::readAbsoluteAngleCounts() {
    uint8_t            buffer[2] = {0, 0};
    TransmissionResult result    = i2c_driver_->readRegister(
        K_I2C_ADDRESS, static_cast<uint8_t>(RegisterAddresses::raw_angle_h), {buffer, 2}, true);
    // TODO what to do if i2c communication fails? For now only assert
    ASSERT(result.status == interfaces::I2CMasterInterface::Status::ok && result.bytes_operated == 2);

    // Combine high and low byte (12-bit value)
    return (static_cast<uint64_t>(buffer[0]) << 8 | buffer[1]) & K_MAX_ABSOLUTE_ANGLE_READING;
}

double AS5600_MagneticEncoderI2cDriver::readAbsoluteAngleDegrees() {
    return (static_cast<double>(readAbsoluteAngleCounts()) * 360.0) / K_MAX_ABSOLUTE_ANGLE_READING;
}

double AS5600_MagneticEncoderI2cDriver::readAbsoluteAngleRadians() {
    return (static_cast<double>(readAbsoluteAngleCounts()) * 2 * std::numbers::pi) / K_MAX_ABSOLUTE_ANGLE_READING;
}

uint64_t AS5600_MagneticEncoderI2cDriver::readScaledAngleCounts() {
    uint8_t            buffer[2] = {0, 0};
    TransmissionResult result =
        i2c_driver_->readRegister(K_I2C_ADDRESS, static_cast<uint8_t>(RegisterAddresses::angle_h), {buffer, 2}, true);
    // TODO what to do if i2c communication fails? For now only assert
    ASSERT(result.status == interfaces::I2CMasterInterface::Status::ok && result.bytes_operated == 2);

    // Combine high and low byte (12-bit value)
    return (static_cast<uint64_t>(buffer[0]) << 8 | buffer[1]) & K_MAX_ABSOLUTE_ANGLE_READING;
}

double AS5600_MagneticEncoderI2cDriver::readScaledAngleDegrees() {
    return (static_cast<double>(readScaledAngleCounts()) * 360.0) / K_MAX_ABSOLUTE_ANGLE_READING;
}

double AS5600_MagneticEncoderI2cDriver::readScaledAngleRadians() {
    return (static_cast<double>(readScaledAngleCounts()) * 2 * std::numbers::pi) / K_MAX_ABSOLUTE_ANGLE_READING;
}

void AS5600_MagneticEncoderI2cDriver::configureMaxAngleForScaledAngle(double degrees) {
    // Convert degrees to raw 12-bit value
    uint16_t raw_value = static_cast<uint16_t>((degrees / 360.0) * K_MAX_ABSOLUTE_ANGLE_READING);
    uint8_t  bytes[2]  = {static_cast<uint8_t>(raw_value >> 8), static_cast<uint8_t>(raw_value & 0xFF)};

    interfaces::I2CMasterInterface::TransmissionResult result =
        i2c_driver_->writeRegister(K_I2C_ADDRESS,
                                   static_cast<uint8_t>(RegisterAddresses::mang_h),  // MSB register
                                   {bytes, 2}, true);
    // TODO what to do if i2c communication fails? For now only assert
    ASSERT(result.status == interfaces::I2CMasterInterface::Status::ok && result.bytes_operated == 2);
}

void AS5600_MagneticEncoderI2cDriver::configureZeroPositionForScaledAngle(uint64_t raw_absolute_angle) {
    // raw_absolute_angle is 12-bit (0..4095)
    uint16_t raw_value = static_cast<uint16_t>(raw_absolute_angle & 0x0FFF);
    uint8_t  bytes[2]  = {static_cast<uint8_t>(raw_value >> 8), static_cast<uint8_t>(raw_value & 0xFF)};

    interfaces::I2CMasterInterface::TransmissionResult result =
        i2c_driver_->writeRegister(K_I2C_ADDRESS,
                                   static_cast<uint8_t>(RegisterAddresses::zpos_h),  // MSB register
                                   {bytes, 2}, true);
    // TODO what to do if i2c communication fails? For now only assert
    ASSERT(result.status == interfaces::I2CMasterInterface::Status::ok && result.bytes_operated == 2);
}

bool AS5600_MagneticEncoderI2cDriver::readConfRegister(uint16_t& buffer) {
    uint8_t            bytes_buffer[2] = {0, 0};
    TransmissionResult result          = i2c_driver_->readRegister(
        K_I2C_ADDRESS, static_cast<uint8_t>(RegisterAddresses::conf_h), {bytes_buffer, 2}, true);

    if (result.status != interfaces::I2CMasterInterface::Status::ok || result.bytes_operated != 2) return false;

    buffer = bytes_buffer[0] << 8 | bytes_buffer[1];
    return true;
}

bool AS5600_MagneticEncoderI2cDriver::writeConfRegister(uint16_t value) {
    uint8_t bytes_buffer[2];
    bytes_buffer[0]           = static_cast<uint8_t>((value >> 8) & 0xFF);  // MSB
    bytes_buffer[1]           = static_cast<uint8_t>(value & 0xFF);         // LSB

    TransmissionResult result = i2c_driver_->writeRegister(
        K_I2C_ADDRESS, static_cast<uint8_t>(RegisterAddresses::conf_h), {bytes_buffer, 2}, true);

    return result.status == interfaces::I2CMasterInterface::Status::ok && result.bytes_operated == 2;
}

}  // namespace drivers::middleware