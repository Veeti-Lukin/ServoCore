#ifndef ANALOGRGBLEDDRIVER_H
#define ANALOGRGBLEDDRIVER_H

#include "drivers/PwmSliceDriver.h"
#include "drivers/interfaces/RgbLedInterface.h"

namespace drivers {

/** @brief Enum to specify the type of RGB LED (Common Anode or Common Cathode). */
enum class AnalogRgbLedType {
    common_anode,
    common_cathode,
};

/**
 * @brief Driver class for controlling an analog RGB LED using PWM channels.
 *
 * This class provides functionality to control an RGB LED using three PWM channels for red, green, and blue.
 * It supports setting the color, brightness, and turning the LED on or off.
 */
class AnalogRgbLedDriver final : public interfaces::RgbLedInterface {
public:
     AnalogRgbLedDriver(AnalogRgbLedType led_type, PwmSliceDriver* red_pwm_slice, PwmChannel red_pwm_channel,
                        PwmSliceDriver* green_pwm_slice, PwmChannel green_pwm_channel, PwmSliceDriver* blue_pwm_slice,
                        PwmChannel blue_pwm_channel);
    ~AnalogRgbLedDriver() = default;

    using interfaces::RgbLedInterface::RGB;

    /**
     * @brief Initialize the PWM slices and set the LED type.
     * This function configures the PWM slices, sets the frequency,
     * and configures the polarity of the channels based on the LED type.
     */
    void init();
    /**
     * @brief De-initialize the PWM slices.
     * This function disables the PWM channels and resets the settings.
     */
    void deInit();

    // public: RgbLedInterface
    /**
     * @brief Turn the RGB LED on.
     * This function enables the PWM channels for red, green, and blue.
     */
    void turnOn() override;
    /**
     * @brief Turn the RGB LED off.
     * This function disables the PWM channels for red, green, and blue.
     */
    void turnOff() override;

    /**
     * @brief Set the color of the RGB LED using individual red, green, and blue components.
     * @param red The red component (0-255).
     * @param green The green component (0-255).
     * @param blue The blue component (0-255).
     */
    void setColorRGB(uint8_t red, uint8_t green, uint8_t blue) override;
    /**
     * @brief Set the color of the RGB LED using an RGB structure.
     * @param rgb The RGB structure containing red, green, and blue components.
     */
    void setColorRGB(RGB rgb) override;

    /**
     * @brief Set the brightness of the RGB LED.
     * @param brightness_percentage The brightness as a percentage (0-100).
     */
    void setBrightness(float brightness_percentage) override;
    /**
     * @brief Get the current brightness of the RGB LED.
     * @return The brightness as a percentage (0-100).
     */
    [[nodiscard]] float getBrightness() const override;

private:
    PwmSliceDriver* red_pwm_slice_   = nullptr;
    PwmSliceDriver* green_pwm_slice_ = nullptr;
    PwmSliceDriver* blue_pwm_slice_  = nullptr;
    PwmChannel      red_pwm_channel_;
    PwmChannel      green_pwm_channel_;
    PwmChannel      blue_pwm_channel_;

    AnalogRgbLedType led_type_;

    float brightness_multiplier_ = 1.0f;
};

}  // namespace drivers

#endif  // ANALOGRGBLEDDRIVER_H
