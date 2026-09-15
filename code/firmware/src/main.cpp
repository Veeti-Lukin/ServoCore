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
#include "drivers/PwmSliceDriver.h"
#include "drivers/SysClockDriver.h"
#include "drivers/TimerDriver.h"
#include "drivers/usb/UsbCdcChannel.h"
#include "drivers/usb/UsbDevice.h"
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

// --------------------------------- USB ---------------------------------
// Composition (two CDC ports) and identity come from hw_mappings; only the names are given here.
hw_mappings::UsbDevice usb_device({.manufacturer   = "Veeti Lukin",
                                   .product        = "ServoCore",
                                   .function_names = {"ServoCore Protocol", "ServoCore Debug"}},
                                  hw_mappings::K_USB_VBUS_DETECT_PIN);
auto&                  usb_protocol_channel = usb_device.channel<hw_mappings::K_USB_PROTOCOL_CHANNEL>();
auto&                  usb_debug_channel    = usb_device.channel<hw_mappings::K_USB_DEBUG_CHANNEL>();

// ----------------------------- COMM PROTOCOL --------------------------------
// The same protocol is served on the field bus UART and on the USB protocol port; both handlers share the
// parameter database.
serial_communication_framework::SlaveHandler protocol_handler(communication_uart_driver, sys_clock_driver, 0);
serial_communication_framework::SlaveHandler usb_protocol_handler(usb_protocol_channel, sys_clock_driver, 0);

// Debug output goes to the USB debug port. The debug UART is kept alongside it until USB is proven on hardware;
// drop the two UART lines below (and the UART itself) after that.
void debugPutChar(char c) {
    debug_uart_driver.transmitByte(c);
    usb_debug_channel.transmitByte(static_cast<uint8_t>(c));
}
void debugFlush() {
    debug_uart_driver.flushTx();
    usb_debug_channel.flushTx();
}

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

    // --------------- INIT USB ---------------
    // The interrupt must be live before init(): enumeration starts the moment the pull-up is presented.
    irq_set_exclusive_handler(hw_mappings::UsbDevice::K_NVIC_INTERRUPT_NUMBER, usbISR);
    irq_set_enabled(hw_mappings::UsbDevice::K_NVIC_INTERRUPT_NUMBER, true);
    usb_device.init();

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

    // --------------- INIT COMMUNICATION ---------------
    protocol_handler.init();
    usb_protocol_handler.init();
}

void registerProtocolCommandHandlers(serial_communication_framework::SlaveHandler& handler) {
    handler.registerCommandHandler<protocol::commands::Ping, protocol_handlers::ping>();
    handler.registerCommandHandler<protocol::commands::GetRegisteredParamIds, protocol_handlers::getParamIds>();
    handler.registerCommandHandler<protocol::commands::GetParamMetadata, protocol_handlers::getParamMetaData>();
    handler.registerCommandHandler<protocol::commands::ReadParamValue, protocol_handlers::readParamValue>();
    handler.registerCommandHandler<protocol::commands::WriteParamValue, protocol_handlers::writeParamValue>();
}

void initSWLibs() {
    debug_print::connectPutCharAndFlushFunctions(&debugPutChar, &debugFlush);
    assert::setAssertionFailedReaction(assert::OnAssertFailReaction::call_assertion_handler_and_break_point);
    assert::connectAssertionFailedHandler(onAssertionFailed);

    registerProtocolCommandHandlers(protocol_handler);
    registerProtocolCommandHandlers(usb_protocol_handler);
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

    parameter_database.registerParameter(&param1);
    parameter_database.registerParameter(&param2);
    parameter_database.registerParameter(&param3);
    parameter_database.registerParameter(&param4);
    parameter_database.registerParameter(&param5);
    parameter_database.registerParameter(&param6);
    parameter_database.registerParameter(&loop_back_parm);

    // USB link health, readable from the dev tool over either port.
    drivers::usb::UsbStats&           usb_stats = usb_device.getStats();
    parameter_system::SignalParameter usb_configured_param(protocol::usb_params::configured, "USB configured",
                                                           usb_stats.configured);
    parameter_system::SignalParameter usb_bus_resets_param(protocol::usb_params::bus_resets, "USB bus resets",
                                                           usb_stats.bus_resets);
    parameter_system::SignalParameter usb_tx_drops_param(protocol::usb_params::tx_drops, "USB TX bytes dropped",
                                                         usb_stats.tx_drops);
    parameter_system::SignalParameter usb_rx_overruns_param(protocol::usb_params::rx_overruns, "USB RX overruns",
                                                            usb_stats.rx_overruns);
    parameter_database.registerParameter(&usb_configured_param);
    parameter_database.registerParameter(&usb_bus_resets_param);
    parameter_database.registerParameter(&usb_tx_drops_param);
    parameter_database.registerParameter(&usb_rx_overruns_param);

    DEBUG_PRINT("Init done, entering main loop!\n");

    /// ************************* MAIN LOOP ************************* ///
    while (true) {
        usb_device.run();  // VBUS housekeeping only; data moves in the interrupt
        protocol_handler.run();
        usb_protocol_handler.run();
        test_uint32++;

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
