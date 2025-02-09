#ifndef LEDCONTROLLER_H
#define LEDCONTROLLER_H

#include "drivers/interfaces/RgbLedInterface.h"

namespace led_controller {

/** @brief Led controller to handle time based effects such as blinking and flashing
 *
 * Led controller supports time based effect generation by calling its periodic update function in either main loop or
 * in timer counter interrupt. The resolution and the accuracy of the effect timing is directly related how usually the
 * periodic update function  is called.
 */
class LedController {
public:
    explicit LedController(drivers::interfaces::RgbLedInterface* rgb_led);
    ~        LedController() = default;

    using RGB                = drivers::interfaces::RgbLedInterface::RGB;

    /**
     * @brief Sets the base color of the LED.
     *
     * The base color is the color that will be shown in between of blinks and
     * after flashing.
     *
     * @param color The RGB color to set as the base color.
     */
    void setConstantBaseColor(const RGB& color);

    /**
     * @brief Temporarily overrides the base color for a short duration.
     * @param color The RGB color to flash.
     * @param duration_ms The duration in milliseconds for which the override color should be shown.
     */
    void flashOverrideColor(const RGB& color, unsigned int duration_ms = 75);

    /**
     * @brief Starts blinking the LED with an override color.
     *        The LED alternates between the override color and the base color.
     * @param color The RGB color to blink.
     * @param symmetrical_duration_ms The duration (in ms) for each state (override and base color).
     */
    void startOverrideColorBlinking(const RGB& color, unsigned int symmetrical_duration_ms);
    /** @brief Stops the blinking effect and restores the base color. */
    void stopOverrideColorBlinking();

    /**
     * @brief Periodically updates LED effects based on elapsed time.
     *
     * Interval of calling this function is directly linked to the accuracy of the effect timings and the resolution of
     * flashes and blinks that can be shown.
     *
     * @param delta_time_ms_since_last_update Time (in ms) since the last update call.
     */
    void periodicUpdate(unsigned int delta_time_ms_since_last_update);

private:
    drivers::interfaces::RgbLedInterface* rgb_led_;

    RGB base_color_{0, 0, 0};
    RGB override_color_{0, 0, 0};

    bool         is_blinking_             = false;
    unsigned int blink_duration_ms_       = 0;
    unsigned int effect_duration_left_ms_ = 0;
    bool         blink_state_             = false;
};

}  // namespace led_controller

#endif  // LEDCONTROLLER_H
