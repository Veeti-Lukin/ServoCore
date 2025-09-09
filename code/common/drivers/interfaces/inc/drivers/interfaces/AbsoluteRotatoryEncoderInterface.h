#ifndef COMMON_DRIVERS_INTERFACES_ENCODERINTERFACE_H
#define COMMON_DRIVERS_INTERFACES_ENCODERINTERFACE_H

#include <cstdint>

namespace drivers::interfaces {

/**
 * @brief Interface for absolute rotary encoders.
 *
 * This interface defines a standard way to interact with absolute encoders that provide
 * raw and scaled angular position readings. The scaled angle can be configured with a
 * custom zero reference and maximum angle to support different application needs,
 * such as partial rotation ranges.
 */
class AbsoluteRotatoryEncoderInterface {
public:
    virtual ~AbsoluteRotatoryEncoderInterface()                                      = default;

    /**
     * @brief Get the maximum possible angle reading in counts.
     *
     * This represents the full-scale resolution of the encoder in raw counts.
     * (e.g., 12-bit -> range [0, 4095] -> max 4095).
     */
    virtual uint64_t getMaxAngleCounts()                                             = 0;

    /**
     * @brief Read the raw absolute angle from the encoder.
     *
     * The raw absolute angle is typically provided as the native resolution
     * of the encoder (e.g., 12-bit -> range [0, 4095]).
     *
     * @return Raw absolute angle as an unsigned integer count.
     */
    virtual uint64_t readAbsoluteAngleCounts()                                       = 0;

    /**
     * @brief Read the absolute angle in degrees.
     *
     * This returns the raw absolute angle scaled to degrees,
     * typically in the range [0.0, 360.0).
     *
     * @return Absolute angle in degrees.
     */
    virtual double readAbsoluteAngleDegrees()                                        = 0;
    /**
     * @brief Read the absolute angle in radians.
     *
     * This returns the raw absolute angle scaled to radians,
     * typically in the range [0.0, 2π).
     *
     * @return Absolute angle in radians.
     */
    virtual double readAbsoluteAngleRadians()                                        = 0;

    /**
     * @brief Read the raw scaled angle from the encoder.
     *
     * The scaled angle represents the position within the configured
     * range (see configureMaxAngleForScaledAngle). It may wrap or saturate
     * depending on the implementation.
     *
     * @return Raw scaled angle as an unsigned integer count.
     */
    virtual uint64_t readScaledAngleCounts()                                         = 0;
    /**
     * @brief Read the scaled angle in degrees.
     *
     * This returns the configured scaled angle in degrees,
     * typically in the range [0.0, max_angle).
     *
     * @return Scaled angle in degrees.
     */
    virtual double readScaledAngleDegrees()                                          = 0;
    /**
     * @brief Read the scaled angle in radians.
     *
     * This returns the configured scaled angle in radians,
     * typically in the range [0.0, max_angle_radians).
     *
     * @return Scaled angle in radians.
     */
    virtual double readScaledAngleRadians()                                          = 0;

    /**
     * @brief Configure the maximum angle for the scaled angle reading.
     *
     * The scaled angle will be mapped into the range [0, degrees),
     * either wrapping around or saturating at the limit depending
     * on the encoder implementation.
     *
     * @param degrees Maximum angle for scaled readings (in degrees).
     */
    virtual void configureMaxAngleForScaledAngle(double degrees)                     = 0;
    /**
     * @brief Configure the zero reference for the scaled angle.
     *
     * Sets the raw absolute angle that corresponds to 0 in the
     * scaled angle domain. This is useful for aligning the encoder
     * reference frame to the mechanical system (e.g., motor shaft).
     *
     * @param absolute_angle_counts Absolute angle integer count to treat as zero.
     */
    virtual void configureZeroPositionForScaledAngle(uint64_t absolute_angle_counts) = 0;
};

}  // namespace drivers::interfaces

#endif  // COMMON_DRIVERS_INTERFACES_ENCODERINTERFACE_H
