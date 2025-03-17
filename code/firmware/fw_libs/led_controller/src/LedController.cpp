#include "led_controller/LedController.h"

namespace led_controller {

LedController::LedController(drivers::interfaces::RgbLedInterface* rgb_led) : rgb_led_(rgb_led) {}

void LedController::setConstantBaseColor(const RGB& color) {
    rgb_led_->setColorRGB(color);
    base_color_ = color;
}

void LedController::flashOverrideColor(const RGB& color, unsigned int duration_ms) {
    rgb_led_->setColorRGB(color);
    effect_duration_left_ms_ = duration_ms;
}

void LedController::startOverrideColorBlinking(const RGB& color, unsigned int symmetrical_duration_ms) {
    override_color_          = color;
    effect_duration_left_ms_ = symmetrical_duration_ms;
    blink_duration_ms_       = symmetrical_duration_ms;
    is_blinking_             = true;
}

void LedController::stopOverrideColorBlinking() {
    is_blinking_             = false;
    effect_duration_left_ms_ = 0;
}

void LedController::periodicUpdate(unsigned int delta_time_ms_since_last_update) {
    if (effect_duration_left_ms_ == 0) return;  // Nothing to do, no time based things happening

    // Ensure no overflow in time calculations
    if (delta_time_ms_since_last_update > effect_duration_left_ms_) {
        delta_time_ms_since_last_update = effect_duration_left_ms_;
    }
    effect_duration_left_ms_ -= delta_time_ms_since_last_update;

    if (is_blinking_ && effect_duration_left_ms_ == 0) {
        // Toggle blink state
        blink_state_ = !blink_state_;
        if (blink_state_) {
            rgb_led_->setColorRGB(override_color_);
        } else {
            rgb_led_->setColorRGB(base_color_);
        }

        // Reset effect duration for the next blink cycle
        effect_duration_left_ms_ = blink_duration_ms_;

        return;
    }

    if (effect_duration_left_ms_ == 0) {
        rgb_led_->setColorRGB(base_color_);
    }
}

}  // namespace led_controller