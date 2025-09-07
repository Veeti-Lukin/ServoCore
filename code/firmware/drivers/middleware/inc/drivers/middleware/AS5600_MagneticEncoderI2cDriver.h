#ifndef FIRMWARE_DRIVERS_MIDDLEWARE_AS5600_MAGNETICENCODERI2CDRIVER_H
#define FIRMWARE_DRIVERS_MIDDLEWARE_AS5600_MAGNETICENCODERI2CDRIVER_H

#include <cstddef>
#include <cstdint>

#include "drivers/interfaces/AbsoluteRotatoryEncoderInterface.h"
#include "drivers/interfaces/I2CMasterInterface.h"

namespace drivers::middleware {

/**
 * @class AS5600_MagneticEncoderI2cDriver
 * @brief Driver for the AS5600 magnetic rotary encoder using I2C communication.
 *
 * This driver provides access to both raw and scaled angle readings,
 * device configuration (power modes, hysteresis, max angle, zero position),
 * and one-time programmable burn commands.
 *
 * It implements the AbsoluteRotatoryEncoderInterface for generic encoder usage.
 */
class AS5600_MagneticEncoderI2cDriver : public interfaces::AbsoluteRotatoryEncoderInterface {
public:
    /**
     * @brief Construct a new AS5600 driver object.
     * @param i2c_driver Pointer to an I2C master interface implementation.
     */
    explicit AS5600_MagneticEncoderI2cDriver(interfaces::I2CMasterInterface* i2c_driver);

    /**
     * @brief Initialize the AS5600 device.
     *
     * Configures the I2C bus speed for communication with the AS5600.
     */
    void init();

    /**
     * @brief Check if the device is connected and responding.
     *
     * Performs a dummy read from a register and checks for I2C ACK.
     * @return true if the device responds, false otherwise.
     */
    bool isConnected();

    /**
     * @struct Status
     * @brief Represents the status of the AS5600 device.
     */
    struct Status {
        bool magnet_detected = false;

        enum class MagnetStrength : uint8_t {
            too_strong,
            too_weak,
            ok,

            unknown,
        };

        MagnetStrength magnet_strength = MagnetStrength::unknown;
    };
    /**
     * @brief Read the magnet detection and strength status.
     * @return Status struct containing magnet detection and field strength info.
     */
    Status readStatus();

    /**
     * @enum BurnCommand
     * @brief Defines permanent one-time programmable EEPROM burn commands.
     */
    enum class BurnCommand : uint8_t {
        burn_angle     = 0x80,  // Burn ZPOS and MPOS (can be run max 3 times per IC)
        burn_max_angle = 0x40   // Burn MANG (can be run only once per IC)
    };
    /**
     * @brief Burn configuration values into OTP memory permanently.
     *
     * WARNING: This action is irreversible and can be executed only
     * a limited number of times.
     *
     * @param command Burn command type.
     * @return true if the operation was acknowledged, false otherwise.
     */
    bool burn(BurnCommand command);

    /**
     * @enum PowerMode
     * @brief Defines power consumption and polling behavior.
     */
    enum class PowerMode : uint8_t {
        normal_operation_mode = 0b00,  ///< Always on, max current consumption ~6.5 mA
        low_power_mode_1      = 0b01,  ///< Polling time 5ms, max current consumption ~3.4 mA
        low_power_mode_2      = 0b10,  ///< Polling time 20ms, max current consumption ~1.8 mA
        low_power_mode_3      = 0b11,  ///< Polling time 100ms, max current consumption ~1.5 mA
    };
    /**
     * @brief Configure power mode.
     * @param mode Desired power mode.
     * @return true if successful, false otherwise.
     */
    bool configurePowerMode(PowerMode mode);

    /**
     * @enum HysteresisMode
     * @brief Defines output hysteresis levels to avoid any toggling of the output when the magnet is not
     * moving, a 1 to 3 LSB hysteresis of the 12 bit resolution
     */
    enum class HysteresisMode : uint8_t {
        off,
        one_lsb,
        two_lsb,
        three_lsb,
    };
    /**
     * @brief Configure output hysteresis.
     *
     * Hysteresis helps suppress output toggling when the magnet is not moving.
     * NOTE: No idea how this works with the automatic low power mode
     *
     * @param mode Desired hysteresis mode.
     * @return true if successful, false otherwise.
     */
    bool configureHysteresis(HysteresisMode mode);

    /**
     * @brief Enable or disable automatic low power mode.
     *
     * If enabled, the device enters low power mode (low_power_mode3)
     * when the angle stays within 4 LSB threshold for at least one minute.
     * NOTE: No idea how this works with the hysteresis
     *
     * @param enable True to enable, false to disable.
     * @return true if successful, false otherwise.
     */
    bool enableAutoLowPowerMode(bool enable);

    /**
     * @enum SlowFilterMode
     * @brief Slow filter configuration.
     *
     * The slow filter is a digital averaging filter that reduces noise and jitter
     * when the magnet is stationary or moving slowly.
     *
     * - Higher values (x16, x8) → smoother readings, better noise immunity,
     *   but slower response time to real angular changes.
     * - Lower values (x4, x2) → faster response, but noisier output.
     */
    enum class SlowFilterMode : uint8_t {
        x16,
        x8,
        x4,
        x2,
    };

    /**
     * @enum FastFilterThreshold
     * @brief Fast filter threshold configuration.
     *
     * The fast filter automatically bypasses the slow filter when angular
     * changes are above a certain threshold, compensating the lag  of the slow filter during fast motion.
     *
     * - Low threshold (e.g. 6 LSB) → minimal lag during fast rotation,
     *   but more sensitive to noise.
     * - High threshold (e.g. 48 LSB) → smoother readings during fast motion,
     *   but slower response to quick angle changes.
     *
     * The threshold is expressed in LSB (least significant bits) of the
     * internal magnitude measurement.
     */
    enum class FastFilterThreshold : uint8_t {
        six_lsb,           ///< Fast override if change > 6 LSB.
        seven_lsb,         ///< Fast override if change > 7 LSB.
        nine_lsb,          ///< Fast override if change > 9 LSB.
        eighteen_lsb,      ///< Fast override if change > 18 LSB.
        twenty_one_lsb,    ///< Fast override if change > 21 LSB.
        twenty_four_lsb,   ///< Fast override if change > 24 LSB.
        twenty_eight_lsb,  ///< Fast override if change > 28 LSB.
        forty_eight_lsb    ///< Fast override if change > 48 LSB.
    };

    /**
     * @brief Configure the noise reduction filters.
     *
     * Combines a slow filter (averaging) with a fast filter (threshold-based override).
     *
     * Typical usage examples:
     * - **Precision sensing / low-noise application** → Slow filter = x16, Fast filter = 48 LSB.
     * - **Fast control loop (e.g. motor commutation)** → Slow filter = x2 or x4, Fast filter = 6 LSB.
     *
     * @param slow_filter_mode    The slow averaging filter setting (SF).
     * @param fast_filter_threshold The fast filter threshold (FTH).
     * @return True if configuration was successfully applied, false otherwise.
     */
    bool configureNoiseReductionFilter(SlowFilterMode slow_filter_mode, FastFilterThreshold fast_filter_threshold);

public:  // AbsoluteRotatoryEncoderInterface
    /** @copydoc AbsoluteRotatoryEncoderInterface::getMaxAngleCounts */
    uint64_t getMaxAngleCounts() override;

    /** @copydoc AbsoluteRotatoryEncoderInterface::readAbsoluteAngleCounts */
    uint64_t readAbsoluteAngleCounts() override;

    /** @copydoc AbsoluteRotatoryEncoderInterface::readAbsoluteAngleDegrees */
    double readAbsoluteAngleDegrees() override;

    /** @copydoc AbsoluteRotatoryEncoderInterface::readAbsoluteAngleRadians */
    double readAbsoluteAngleRadians() override;

    /** @copydoc AbsoluteRotatoryEncoderInterface::readScaledAngleCounts */
    uint64_t readScaledAngleCounts() override;

    /** @copydoc AbsoluteRotatoryEncoderInterface::readScaledAngleDegrees */
    double readScaledAngleDegrees() override;

    /** @copydoc AbsoluteRotatoryEncoderInterface::readScaledAngleRadians */
    double readScaledAngleRadians() override;

    /** @copydoc AbsoluteRotatoryEncoderInterface::configureMaxAngleForScaledAngle */
    void configureMaxAngleForScaledAngle(double degrees) override;

    /** @copydoc AbsoluteRotatoryEncoderInterface::configureZeroPositionForScaledAngle */
    void configureZeroPositionForScaledAngle(uint64_t raw_absolute_angle) override;

private:
    static constexpr uint8_t  K_I2C_ADDRESS                = 0x36;
    static constexpr size_t   K_I2C_BITRATE                = 400'000;  // TODO is this good speed?
    static constexpr uint16_t K_MAX_ABSOLUTE_ANGLE_READING = 4095;     // 12 bit resolution 2^12-1
    static constexpr uint16_t K_I2C_TIMEOUT_US             = 1'000;    // TODO calculate this from the i2c speed?
    // clang-format off
    // Each register is 1 byte or 8 bit wide
    enum class RegisterAddresses : uint8_t {
        zmco   = 0x00,  // Burn count (R)
        zpos_h = 0x01,  // Start position MSB (R/W)
        zpos_l = 0x02,  // Start position LSB (R/W)
        mpos_h = 0x03,  // Stop position MSB (R/W)
        mpos_l = 0x04,  // Stop position LSB (R/W)
        mang_h = 0x05,  // Max angle MSB (R/W)
        mang_l = 0x06,  // Max angle LSB (R/W)
        conf_h = 0x07,  // Configuration MSB (R/W)
        conf_l = 0x08,  // Configuration LSB (R/W)

        // Note: 0x09–0x0A are test/unused (don’t touch)

        raw_angle_h = 0x0C,  // Raw angle MSB (R, 12-bit, no address pointer auto-increment on read, but supports multibyte read to read high and low byte on same transaction)
        raw_angle_l = 0x0D,  // Raw angle LSB (R)
        angle_h     = 0x0E,  // Scaled angle MSB (R, 12-bit, no address pointer auto-increment on read, but supports multibyte read to read high and low byte on same transaction)
        angle_l     = 0x0F,  // Scaled angle LSB (R)

        status      = 0x0B,  // Status (R) → magnet too strong/weak/OK
        agc         = 0x1A,  // Automatic Gain Control (R)
        magnitude_h = 0x1B,  // Magnitude MSB (R, 12-bit, no address pointer auto-increment on read, but supports multibyte read to read high and low byte on same transaction)
        magnitude_l = 0x1C,  // Magnitude LSB (R)

        burn        = 0xFF  // Burn command (W) → one-time OTP programming
    };
    // clang-format on

    enum class StatusRegisterBits : uint8_t {
        magnet_detected   = 1 << 5,
        magnet_too_weak   = 1 << 4,
        magnet_too_strong = 1 << 3,
    };

    enum class ConfRegisterBits : uint16_t {
        pm1   = 1 << 0,
        pm2   = 1 << 1,
        hyst1 = 1 << 2,
        hyst2 = 1 << 3,
        outs1 = 1 << 4,
        outs2 = 1 << 5,
        pwmf1 = 1 << 6,
        pwmf2 = 1 << 7,
        sf1   = 1 << 8,
        sf2   = 1 << 9,
        fth1  = 1 << 10,
        fth2  = 1 << 11,
        fth3  = 1 << 12,
        wd    = 1 << 13,
    };

    bool readConfRegister(uint16_t& buffer);
    bool writeConfRegister(uint16_t value);

    interfaces::I2CMasterInterface* i2c_driver_;
};

}  // namespace drivers::middleware

#endif  // FIRMWARE_DRIVERS_MIDDLEWARE_AS5600_MAGNETICENCODERI2CDRIVER_H
