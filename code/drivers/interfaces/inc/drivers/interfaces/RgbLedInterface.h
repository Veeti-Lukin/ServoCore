#ifndef RGBLEDINTERFACE_H
#define RGBLEDINTERFACE_H

#include <cstdint>

namespace drivers::interfaces {

/**
 * @brief Interface class for controlling RGB LEDs.
 *
 * This interface defines the basic functionality for controlling RGB LEDs, including
 * turning them on/off, setting their color, and adjusting brightness.
 */
class RgbLedInterface {
public:
    virtual ~RgbLedInterface() = default;

    /** @brief Structure to represent RGB color values. */
    struct RGB {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
    };

    /**
     * @brief Turn the RGB LED on.
     * This function should turn the LED on with the current color settings.
     */
    virtual void turnOn()                                              = 0;
    /**
     * @brief Turn the RGB LED off.
     * This function should turn the LED off.
     */
    virtual void turnOff()                                             = 0;

    /**
     * @brief Set the color of the RGB LED using individual RGB components.
     * @param red The red component (0-255).
     * @param green The green component (0-255).
     * @param blue The blue component (0-255).
     */
    virtual void setColorRGB(uint8_t red, uint8_t green, uint8_t blue) = 0;
    /**
     * @brief Set the color of the RGB LED using an RGB structure.
     * @param rgb The RGB structure containing red, green, and blue components.
     */
    virtual void setColorRGB(RGB rgb)                                  = 0;

    /**
     * @brief Set the brightness of the RGB LED.
     * @param brightness_percentage The brightness value as a percentage (0-100).
     */
    virtual void setBrightness(float brightness_percentage)            = 0;
    /**
     * @brief Get the current brightness of the RGB LED.
     * @return The current brightness as a percentage (0-100).
     */
    [[nodiscard]] virtual float getBrightness() const                  = 0;
};

}  // namespace drivers::interfaces

#endif  // RGBLEDINTERFACE_H
