#ifndef PWMDRIVER_H
#define PWMDRIVER_H

#include <cstdint>

namespace drivers {

/** @brief Enum representing the PWM channels. */
enum class PwmChannel : uint8_t { A = 0, B = 1 };

/**
 * @brief Maps an index to a PWM channel.
 *
 * @param index The index (0 or 1).
 * @return PwmChannel The corresponding PWM channel.
 */
PwmChannel mapIndexToPwmChannel(const unsigned int index);
/**
 * @brief Maps a PWM channel to its index.
 *
 * @param channel The PWM channel.
 * @return unsigned int The corresponding index.
 */
unsigned int mapPwmChannelToIndex(PwmChannel channel);

//
//
//
//
//
//

// TODO ADD THE PHASE CORRECT MODE
// and possibly other functionality that hw pwm supports

/**
 * @brief PWM Slice Driver class to manage PWM slices.
 *
 * Each slice can drive two PWM output signals, or measure the frequency
 * or duty cycle of an input signal. The two outputs on each slice have the same period, but independently varying duty
 * cycles.
 *
 * Corresponding slice indexes and channel indexes can be found using pico sdk functions:
 *      -pwm_gpio_to_slice_num()
 *      -pwm_gpio_to_channel()
 */
class PwmSliceDriver {
public:
    explicit PwmSliceDriver(unsigned int slice_index);
    ~        PwmSliceDriver() = default;

    /** @brief Initialize the PWM slice with default settings. */
    void init();
    /** @brief Deinitializes the PWM slice, restoring default settings. */
    void deInit();

    /** @brief Enables the PWM slice. */
    void enable();
    /** @brief Disables the PWM slice. */
    void disable();

    /**
     * @brief Sets the PWM frequency.
     *
     * @param frequency The desired PWM frequency in Hz.
     */
    void setFrequency(unsigned int frequency);
    /**
     * @brief Gets the current PWM frequency.
     *
     * @return unsigned int The current PWM frequency in Hz.
     */
    [[nodiscard]] unsigned int getFrequency() const;

    /**
     * @brief Sets the duty cycle for a given channel.
     *
     * @param channel The PWM channel (A or B).
     * @param duty_cycle_percentage The duty cycle percentage (0-100%).
     */
    void setChannelDutyCycle(PwmChannel channel, float duty_cycle_percentage);
    /**
     * @brief Gets the duty cycle for a given channel.
     *
     * @param channel The PWM channel (A or B).
     * @return float The duty cycle percentage (0-100%).
     */
    [[nodiscard]] float getChannelDutyCycle(PwmChannel channel) const;

    /**
     * @brief Sets the polarity inversion for a given PWM channel.
     *
     * @param channel The PWM channel (A or B).
     * @param inverted True to invert polarity, false for normal operation.
     */
    void setChannelPolarityInverted(PwmChannel channel, bool inverted);

private:
    unsigned int slice_index_;

    uint16_t getWrap() const;
    // AKA. channel level in pico sdk
    uint16_t getCounterCompareValue(PwmChannel channel) const;

    float getClockDivider() const;
};

}  // namespace drivers

#endif  // PWMDRIVER_H
