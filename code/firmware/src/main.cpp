#include <hardware/gpio.h>
#include <hardware/irq.h>
#include <hardware/pwm.h>
#include <hardware/structs/uart.h>
#include <hardware/uart.h>

#include <cmath>

#include "assert/assert.h"
#include "debug_print/debug_print.h"
#include "drivers/AnalogRgbLedDriver.h"
#include "drivers/BufferedAsyncUartDriver.h"
#include "drivers/I2CMasterDriver.h"
#include "drivers/PwmSliceDriver.h"
#include "drivers/SysClockDriver.h"
#include "drivers/TimerDriver.h"
#include "drivers/middleware/AS5600_MagneticEncoderI2cDriver.h"
#include "hw_mappings.h"
#include "interrupt_service_routines.h"
#include "led_controller/LedController.h"
#include "led_controller/common_colors.h"
#include "parameter_system/ParameterDatabase.h"
#include "parameter_system/definition_helpers.h"
#include "protocol/commands.h"
#include "protocol/parameters.h"
#include "protocol_handlers.h"
#include "serial_communication_framework/SlaveHandler.h"
#include "utils/RingBuffer.h"

namespace uart_config = drivers::uart_config;
// -------------------------------- GENERAL -------------------------------
drivers::SysClockDriver sys_clock_driver;

// ---------------------- SERIAL COMMUNICATION UART -----------------------
utils::RingBuffer<128>           communication_uart_tx_buffer;
utils::RingBuffer<128>           communication_uart_rx_buffer;
drivers::BufferedAsyncUartDriver communication_uart_driver(hw_mappings::K_SERIAL_COMMUNICATION_UART_INSTANCE,
                                                           &communication_uart_tx_buffer, &communication_uart_rx_buffer,
                                                           115200, uart_config::DataBits::eight,
                                                           uart_config::StopBits::one, uart_config::Parity::none);

// ------------------------------ DEBUG UART ------------------------------
utils::RingBuffer<128>           debug_uart_tx_buffer;
utils::RingBuffer<128>           debug_uart_rx_buffer;
drivers::BufferedAsyncUartDriver debug_uart_driver(hw_mappings::K_DEBUG_UART_INSTANCE, &debug_uart_tx_buffer,
                                                   &debug_uart_rx_buffer, 115200, uart_config::DataBits::eight,
                                                   uart_config::StopBits::one, uart_config::Parity::none);

// --------------------------------- LED ---------------------------------
drivers::PwmSliceDriver     red_slice_driver(pwm_gpio_to_slice_num(hw_mappings::K_STATUS_LED_RED_PIN));
drivers::PwmSliceDriver     green_slice_driver(pwm_gpio_to_slice_num(hw_mappings::K_STATUS_LED_GREEN_PIN));
drivers::PwmSliceDriver     blue_slice_driver(pwm_gpio_to_slice_num(hw_mappings::K_STATUS_LED_BLUE_PIN));
drivers::AnalogRgbLedDriver led_driver(
    drivers::AnalogRgbLedType::common_cathode, &red_slice_driver,
    drivers::mapIndexToPwmChannel(pwm_gpio_to_channel(hw_mappings::K_STATUS_LED_RED_PIN)), &green_slice_driver,
    drivers::mapIndexToPwmChannel(pwm_gpio_to_channel(hw_mappings::K_STATUS_LED_GREEN_PIN)), &blue_slice_driver,
    drivers::mapIndexToPwmChannel(pwm_gpio_to_channel(hw_mappings::K_STATUS_LED_BLUE_PIN)));
led_controller::LedController status_led_controller(&led_driver);

drivers::TimerDriver led_update_timer(hw_mappings::K_PERIODIC_LED_TIMER_INSTANCE,
                                      hw_mappings::K_PERIODIC_LED_TIMER_ALARM_CHANNEL);
// ----------------------------- PARAMETER SYSTEM ------------------------------
parameter_system::ParameterDefinition* parameter_buffer[64] = {nullptr};
parameter_system::ParameterDatabase    parameter_database({parameter_buffer});

// ----------------------------- COMM PROTOCOL --------------------------------
serial_communication_framework::SlaveHandler protocol_handler(communication_uart_driver, sys_clock_driver, 0);

// ------------------------------- ENCODER ----------------------------------
drivers::I2CMasterDriver                             encoder_i2c(i2c1, 100'000);
drivers::middleware::AS5600_MagneticEncoderI2cDriver encoder(&encoder_i2c);

void debugUartPutChar(char c) { debug_uart_driver.transmitByte(c); }
void DebugUartFlush() { debug_uart_driver.flushTx(); }

void onAssertionFailed() { status_led_controller.setConstantBaseColor(led_controller::common_colors::K_RED); }

void initHW() {
    // --------------- INIT UART ---------------
    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select

    gpio_set_function(hw_mappings::K_SERIAL_COMMUNICATION_UART_TX_PIN,
                      UART_FUNCSEL_NUM(hw_mappings::K_SERIAL_COMMUNICATION_UART_INSTANCE,
                                       hw_mappings::K_SERIAL_COMMUNICATION_UART_TX_PIN));
    gpio_set_function(hw_mappings::K_SERIAL_COMMUNICATION_UART_RX_PIN,
                      UART_FUNCSEL_NUM(hw_mappings::K_SERIAL_COMMUNICATION_UART_INSTANCE,
                                       hw_mappings::K_SERIAL_COMMUNICATION_UART_RX_PIN));

    gpio_set_function(hw_mappings::K_DEBUG_UART_TX_PIN,
                      UART_FUNCSEL_NUM(hw_mappings::K_DEBUG_UART_INSTANCE, hw_mappings::K_DEBUG_UART_TX_PIN));
    gpio_set_function(hw_mappings::K_DEBUG_UART_RX_PIN,
                      UART_FUNCSEL_NUM(hw_mappings::K_DEBUG_UART_INSTANCE, hw_mappings::K_DEBUG_UART_RX_PIN));

    irq_set_exclusive_handler(communication_uart_driver.getNvicCombinedUartInterruptNumber(),
                              serialCommunicationUartCombinedISR);
    // Enable the UART IRQ in the NVIC
    irq_set_enabled(communication_uart_driver.getNvicCombinedUartInterruptNumber(), true);
    irq_set_exclusive_handler(debug_uart_driver.getNvicCombinedUartInterruptNumber(), debugUartCombinedISR);
    // Enable the UART IRQ in the NVIC
    irq_set_enabled(debug_uart_driver.getNvicCombinedUartInterruptNumber(), true);

    debug_uart_driver.init();
    communication_uart_driver.init();

    // --------------- INIT LED PWM ---------------
    gpio_set_function(hw_mappings::K_STATUS_LED_RED_PIN, GPIO_FUNC_PWM);
    gpio_set_function(hw_mappings::K_STATUS_LED_GREEN_PIN, GPIO_FUNC_PWM);
    gpio_set_function(hw_mappings::K_STATUS_LED_BLUE_PIN, GPIO_FUNC_PWM);
    // initializes the PWM:s for color control
    led_driver.init();
    led_driver.turnOn();
    led_driver.setBrightness(5);

    // --------------- INIT LED TIMER ---------------
    led_update_timer.configureInMilliseconds(20);  // Lowering this increases the accuracy of led effects
    irq_set_exclusive_handler(led_update_timer.getIrqNumber(), periodicLedUpdateTimerISR);
    irq_set_enabled(led_update_timer.getIrqNumber(), true);
    led_update_timer.start();

    // ------------------ INIT ENCODER ------------------
    gpio_set_function(hw_mappings::K_ENCODER_READER_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_set_function(hw_mappings::K_ENCODER_READER_I2C_SDA_PIN, GPIO_FUNC_I2C);
    // These pullups substitute during development time but on the PCBA there should be externals
    gpio_pull_up(hw_mappings::K_ENCODER_READER_I2C_SCL_PIN);
    gpio_pull_up(hw_mappings::K_ENCODER_READER_I2C_SDA_PIN);
    encoder_i2c.init();
    encoder.init();  // < Sets the correct i2c speed etc

    // --------------- INIT COMMUNICATION ---------------
    protocol_handler.init();
}

void initSWLibs() {
    debug_print::connectPutCharAndFlushFunctions(&debugUartPutChar, &DebugUartFlush);
    assert::setAssertionFailedReaction(assert::OnAssertFailReaction::call_assertion_handler_and_break_point);
    assert::connectAssertionFailedHandler(onAssertionFailed);

    protocol_handler.registerCommandHandler<protocol::commands::Ping, protocol_handlers::ping>();
    protocol_handler
        .registerCommandHandler<protocol::commands::GetRegisteredParamIds, protocol_handlers::getParamIds>();
    protocol_handler
        .registerCommandHandler<protocol::commands::GetParamMetadata, protocol_handlers::getParamMetaData>();
    protocol_handler.registerCommandHandler<protocol::commands::ReadParamValue, protocol_handlers::readParamValue>();
    protocol_handler.registerCommandHandler<protocol::commands::WriteParamValue, protocol_handlers::writeParamValue>();
}

[[noreturn]] int main() {
    initHW();  // TODO can sw libs be initialized first? At least assert lib should be but uart should be initialized
               // for that
    initSWLibs();

    DEBUG_PRINT("Starting up!\n");
    status_led_controller.setConstantBaseColor(led_controller::common_colors::K_YELLOW);

    uint8_t  test_uint8  = 42;
    uint16_t test_uint16 = 1337;
    uint32_t test_uint32 = 123456;
    float    test_float  = 3.1415;
    bool     test_bool   = true;
    uint64_t test_uint64 = 123456;

    parameter_system::SavedParameter   param1(protocol::test_params::test_uint8, "Test Uint8", test_uint8);
    parameter_system::SignalParameter  param2(protocol::test_params::test_uint16, "Test Uint16", test_uint16);
    parameter_system::SignalParameter  param3(protocol::test_params::test_uint32, "Test Uint32", test_uint32);
    parameter_system::SignalParameter  param4(protocol::test_params::test_float, "Test Float", test_float);
    parameter_system::RuntimeParameter param5(protocol::test_params::test_bool, "Test Bool", test_bool);
    parameter_system::RuntimeParameter param6(protocol::test_params::test_test, "Test U64", test_uint64);
    parameter_system::SignalParameter  loop_back_parm(protocol::test_params::loop_back, "Loopback of Test Uint8",
                                                      test_uint8);

    double                            encoder_angle_degrees = 0.0;
    parameter_system::SignalParameter encoder_angle_degrees_param(protocol::motor_params::encoder_angle_degrees,
                                                                  "Encoder Angle (deg)", encoder_angle_degrees);

    parameter_database.registerParameter(&param1);
    parameter_database.registerParameter(&param2);
    parameter_database.registerParameter(&param3);
    parameter_database.registerParameter(&param4);
    parameter_database.registerParameter(&param5);
    parameter_database.registerParameter(&param6);
    parameter_database.registerParameter(&loop_back_parm);
    parameter_database.registerParameter(&encoder_angle_degrees_param);

    DEBUG_PRINT("Init done, entering main loop!\n");

    /// ************************* MAIN LOOP ************************* ///
    while (true) {
        protocol_handler.run();
        test_uint32++;

        encoder_angle_degrees = encoder.readAbsoluteAngleDegrees();

        /* // Old debugging code that can be removed later
        while (communication_uart_driver.getReceivedBytesAvailableAmount() > 0) {
            status_led_controller.flashOverrideColor(led_controller::common_colors::K_ORANGE);
            communication_uart_driver.transmitByte(communication_uart_driver.readReceivedByte());
        }

        while (debug_uart_driver.getReceivedBytesAvailableAmount() > 0) {
            status_led_controller.flashOverrideColor(led_controller::common_colors::K_GREEN);
            debug_uart_driver.transmitByte(debug_uart_driver.readReceivedByte());
        }*/
    }

    return 0;
}
