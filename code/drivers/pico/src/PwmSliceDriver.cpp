#include "drivers/PwmSliceDriver.h"

#include <hardware/clocks.h>
#include <hardware/pwm.h>
#include <hardware/resets.h>

#include <cmath>
#include <cstdint>
#include <limits>

#include "assert/assert.h"
#include "debug_print/debug_print.h"

namespace drivers {

// The wrap register is 16 bits.
// Wrap register is where pwm counter resets to zero or if phase correct is enabled starts counting down
unsigned int K_WRAP_REGISTER_MAX = std::numeric_limits<uint16_t>::max();

unsigned int K_CLOCK_DIVIDER_MAX = 256;
unsigned int K_CLOCK_DIVIDER_MIN = 1;

PwmChannel mapIndexToPwmChannel(const unsigned int index) {
    if (index == 0) return PwmChannel::A;
    if (index == 1) return PwmChannel::B;
    ASSERT(index == 1 || index == 0);
    return PwmChannel::A;
}

unsigned int mapPwmChannelToIndex(PwmChannel channel) { return static_cast<unsigned int>(channel); }

PwmSliceDriver::PwmSliceDriver(const unsigned int slice_index) : slice_index_(slice_index) {}

void PwmSliceDriver::init() {
    pwm_config default_config = pwm_get_default_config();
    // Restore default settings for the pwm slice and init to disabled state
    pwm_init(slice_index_, &default_config, false);
    // Redundant to config but added here for readability
    pwm_set_enabled(slice_index_, false);                 // Start disabled
    pwm_set_clkdiv(slice_index_, 1.0f);                   // Default clock divider
    pwm_set_wrap(slice_index_, 65535);                    // Default wrap value (max resolution)
    pwm_set_output_polarity(slice_index_, false, false);  // Start both channels in non-inverted mode
}

void PwmSliceDriver::deInit() {
    pwm_config default_config = pwm_get_default_config();
    // Restore default settings for the pwm slice and init to disabled state
    pwm_init(slice_index_, &default_config, false);
}

void PwmSliceDriver::enable() { pwm_set_enabled(slice_index_, true); }

void PwmSliceDriver::disable() { pwm_set_enabled(slice_index_, false); }

void PwmSliceDriver::setFrequency(unsigned int frequency) {
    // TODO assert frequency > 0
    if (frequency == 0) frequency = 1;

    // System clock frequency
    unsigned int clock_freq = clock_get_hz(clk_sys);  // 125 MHz by default on rp2040 and 150 MHz on rp2350

    // Preserve current duty cycles
    float channel_a_duty    = getChannelDutyCycle(PwmChannel::A);
    float channel_b_duty    = getChannelDutyCycle(PwmChannel::B);

    /*
    As already stated, the wrap value determines the frequency. It doesn’t take too much algebra to work out that in
    non-phase-correct mode:

    f = fc /(wrap+1)

    where fc is the frequency of the clock after any division has been taken into account. More usefully you can work
    out the wrap needed to give any PWM frequency:

    wrap = fc/f - 1

    Similarly, the level sets the duty cycle:

    level = wrap * duty

    The PWM counter is 16 bits and this means that once you reach a wrap of 65,534 you cannot decrease the frequency.
    Given a clock input of 125MHz this puts the lowest PWM frequency at 1.9kHz. This isn’t much good if you need a 50Hz
    signal to drive a servo, see later. The solution is to use the clock divider to reduce the 125MHz clock to something
    lower.

    The clock divider is a 16-bit fractional divider with eight bits for the integer part and four bits for the
    fraction. You can set the clock using:

    pwm_set_clkdiv_int_frac (uint slice_num,
                        uint8_t integer, uint8_t fract)

    and if you want to specify the divider as a floating point value you can use:

    pwm_set_clkdiv (uint slice_num, float divider)

    The divider is decomposed into an 8-bit integer and a 4-bit fractional part, which specifies the fraction as
    fract/16. This means that the largest divisor is 255 15/16 which which gives the lowest clock frequency as:

    522,875.81Hz

    You can use the formulas listed above to work out the wrap and level as long as fc is the resulting clock after
    division. For example, if you set a clock divider of 2 with a clock frequency of 125MHz in non-phase-correct mode
    you can generate a PWM signal of 5kHz using a wrap of 12,500 and a 50% duty cycle implies a level of 6,250.

    This is simple enough, but notice that you now usually have more than one way to generate any particular PWM
    frequency – how should you choose a clock divider? The answer is to do with the resolution of the duty cycle you can
    set. For example, suppose you select a divider that means that to get the frequency you want you have to use a wrap
    of 2. Then the only duty cycles you can set are 0, 1/3, 2/3 or 100%, corresponding to levels of 0, 1, 2 and 3. If
    you want to set more duty cycles then clearly you need to keep wrap as big as possible. In fact, it is easy to see
    that the resolution in bits of the duty cycle is given by log2 wrap and, as the maximum value of wrap is 65,535,
    obviously the maximum resolution is 16 bits, i.e. the size of the counter.

    Putting all this together you can see that you should always choose a divider that lets you use the largest value of
    wrap to generate the frequency you are looking for.

    In other words, the divider should be chosen to be bigger than the frequency you need, but as close as possible. In
    other words:
        divider = Ceil(16fc/65536fpwm)/16

        divider = Ceil(fc/4096fpwm)/16
    */

    // TODO implement with looping would be probably easier to check for too low frequencies

    // Calculate optimal clock divider and wrap value
    float clk_div           = static_cast<float>(clock_freq) / (frequency * static_cast<float>(K_WRAP_REGISTER_MAX));

    if (clk_div < K_CLOCK_DIVIDER_MIN) clk_div = K_CLOCK_DIVIDER_MIN;
    if (clk_div > K_CLOCK_DIVIDER_MAX) clk_div = K_CLOCK_DIVIDER_MAX;

    uint32_t wrap_value = static_cast<uint32_t>((clock_freq / (clk_div * frequency)) - 1);

    // Ensure wrap is within limits
    if (wrap_value > K_WRAP_REGISTER_MAX) wrap_value = K_WRAP_REGISTER_MAX;
    if (wrap_value < 0) wrap_value = 1;  // Prevent wrap=0 causing division by zero

    // Apply calculated values
    pwm_set_clkdiv(slice_index_, clk_div);
    pwm_set_wrap(slice_index_, wrap_value);

    DEBUG_PRINT("WRAP : % \n", wrap_value);

    // Re-set the channel duties because the duty cycle would otherwise be affected by changes to the top/wrap register
    setChannelDutyCycle(PwmChannel::A, channel_a_duty);
    setChannelDutyCycle(PwmChannel::B, channel_b_duty);
}

unsigned int PwmSliceDriver::getFrequency() const {
    // System clock frequency
    // 125 MHz by default on rp2040 and 150 MHz on rp2350
    uint32_t clock_freq = clock_get_hz(clk_sys);

    // Get the current clock divider and wrap value
    float    clk_div    = getClockDivider();
    uint16_t wrap       = getWrap();

    // Prevent division by zero
    if (clk_div == 0.0f || wrap == 0) {
        return 0;  // Invalid configuration
    }

    // Calculate frequency using: f = fc / (clk_div * (wrap + 1))
    return static_cast<unsigned int>(clock_freq / (clk_div * (wrap + 1)));
}

void PwmSliceDriver::setChannelDutyCycle(const PwmChannel channel, float duty_cycle_percentage) {
    if (duty_cycle_percentage > 100.0f) duty_cycle_percentage = 100.0f;
    if (duty_cycle_percentage < 0.0f) duty_cycle_percentage = 0.0f;

    // Get the wrap value (TOP) for this slice
    uint16_t top = getWrap();

    pwm_set_chan_level(slice_index_, mapPwmChannelToIndex(channel), std::round(top * duty_cycle_percentage / 100.0f));
}

float PwmSliceDriver::getChannelDutyCycle(PwmChannel channel) const {
    // Get the wrap value (TOP) for this slice
    uint16_t top                   = getWrap();

    uint16_t counter_compare_value = getCounterCompareValue(channel);

    // Calculate and return the duty cycle percentage
    return (static_cast<float>(counter_compare_value) / top) * 100.0f;
}

void PwmSliceDriver::setChannelPolarityInverted(PwmChannel channel, bool inverted) {
    // Read the current CSR register value
    uint32_t reg   = pwm_hw->slice[slice_index_].csr;

    // Determine which bit to modify
    uint32_t mask  = (channel == PwmChannel::A) ? PWM_CH0_CSR_A_INV_BITS : PWM_CH0_CSR_B_INV_BITS;
    uint32_t value = (inverted ? mask : 0);

    // Update only the selected channel while preserving the other channel's state
    hw_write_masked(&pwm_hw->slice[slice_index_].csr, value, mask);
}

// ------------------------------ PRIVATE METHODS ------------------------------

uint16_t PwmSliceDriver::getWrap() const { return pwm_hw->slice[slice_index_].top; }

uint16_t PwmSliceDriver::getCounterCompareValue(PwmChannel channel) const {
    uint32_t cc_value = pwm_hw->slice[slice_index_].cc;

    if (channel == PwmChannel::A) {
        return (cc_value >> PWM_CH0_CC_A_LSB) & ((1 << PWM_CH0_CC_A_BITS) - 1);
    }
    if (channel == PwmChannel::B) {
        return (cc_value >> PWM_CH0_CC_B_LSB) & ((1 << PWM_CH0_CC_B_BITS) - 1);
    }
    // Should not happen
    ASSERT(channel == PwmChannel::A || channel == PwmChannel::B);

    return 0;
}

float PwmSliceDriver::getClockDivider() const {
    // Read the raw clock divider register
    uint32_t reg      = pwm_hw->slice[slice_index_].div;

    // Extract integer and fractional parts
    uint8_t int_part  = (reg & PWM_CH0_DIV_INT_BITS) >> PWM_CH0_DIV_INT_LSB;
    uint8_t frac_part = (reg & PWM_CH0_DIV_FRAC_BITS) >> PWM_CH0_DIV_FRAC_LSB;

    // Compute the final divider value
    return int_part + (frac_part / 16.0f);
}

}  // namespace drivers