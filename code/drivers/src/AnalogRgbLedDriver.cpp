#include "drivers/AnalogRgbLedDriver.h"

#include <limits>

namespace drivers {

unsigned int K_LED_PWM_FREQUENCY = 20'000;  // Outside the hearing range

AnalogRgbLedDriver::AnalogRgbLedDriver(AnalogRgbLedType led_type, PwmSliceDriver* red_pwm_slice,
                                       PwmChannel red_pwm_channel, PwmSliceDriver* green_pwm_slice,
                                       PwmChannel green_pwm_channel, PwmSliceDriver* blue_pwm_slice,
                                       PwmChannel blue_pwm_channel)
    : red_pwm_slice_(red_pwm_slice),
      green_pwm_slice_(green_pwm_slice),
      blue_pwm_slice_(blue_pwm_slice),
      red_pwm_channel_(red_pwm_channel),
      green_pwm_channel_(green_pwm_channel),
      blue_pwm_channel_(blue_pwm_channel),
      led_type_(led_type) {}

void AnalogRgbLedDriver::init() {
    red_pwm_slice_->init();
    green_pwm_slice_->init();
    blue_pwm_slice_->init();

    red_pwm_slice_->setFrequency(K_LED_PWM_FREQUENCY);
    green_pwm_slice_->setFrequency(K_LED_PWM_FREQUENCY);
    blue_pwm_slice_->setFrequency(K_LED_PWM_FREQUENCY);

    if (led_type_ == AnalogRgbLedType::common_anode) {
        red_pwm_slice_->setChannelPolarityInverted(red_pwm_channel_, true);
        green_pwm_slice_->setChannelPolarityInverted(green_pwm_channel_, true);
        blue_pwm_slice_->setChannelPolarityInverted(blue_pwm_channel_, true);
    }
}

void AnalogRgbLedDriver::deInit() {
    red_pwm_slice_->deInit();
    green_pwm_slice_->deInit();
    blue_pwm_slice_->deInit();
}

void AnalogRgbLedDriver::turnOn() {
    red_pwm_slice_->enable();
    green_pwm_slice_->enable();
    blue_pwm_slice_->enable();
}

void AnalogRgbLedDriver::turnOff() {
    red_pwm_slice_->disable();
    green_pwm_slice_->disable();
    blue_pwm_slice_->disable();
}

void AnalogRgbLedDriver::setColorRGB(uint8_t red, uint8_t green, uint8_t blue) {
    const float red_duty = static_cast<float>(red) / static_cast<float>(std::numeric_limits<uint8_t>::max()) * 100.0f *
                           brightness_multiplier_;
    const float green_duty = static_cast<float>(green) / static_cast<float>(std::numeric_limits<uint8_t>::max()) *
                             100.0f * brightness_multiplier_;
    const float blue_duty = static_cast<float>(blue) / static_cast<float>(std::numeric_limits<uint8_t>::max()) *
                            100.0f * brightness_multiplier_;

    red_pwm_slice_->setChannelDutyCycle(red_pwm_channel_, red_duty);
    green_pwm_slice_->setChannelDutyCycle(blue_pwm_channel_, green_duty);
    blue_pwm_slice_->setChannelDutyCycle(blue_pwm_channel_, blue_duty);
}

void AnalogRgbLedDriver::setColorRGB(const RGB rgb) { setColorRGB(rgb.red, rgb.green, rgb.blue); }

void AnalogRgbLedDriver::setBrightness(float brightness_percentage) {
    if (brightness_percentage > 100) brightness_percentage = 100;
    if (brightness_percentage < 0) brightness_percentage = 0;

    brightness_multiplier_ = brightness_percentage / 100.0f;
}

float AnalogRgbLedDriver::getBrightness() const { return brightness_multiplier_ * 100.0f; }

}  // namespace drivers