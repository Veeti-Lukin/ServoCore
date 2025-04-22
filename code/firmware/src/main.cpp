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
#include "hw_mappings.h"
#include "interrupt_service_routines.h"
#include "led_controller/LedController.h"
#include "led_controller/common_colors.h"
#include "parameter_system/ParameterDatabase.h"
#include "parameter_system/ParameterDelegate.h"
#include "protocol/commands.h"
#include "protocol/requests.h"
#include "protocol_handlers.h"
#include "serial_communication_framework/SlaveHandler.h"
#include "utils/RingBuffer.h"

#define UART_ID   uart0
#define BAUD_RATE 115200

namespace uart_config = drivers::uart_config;
// -------------------------------- GENERAL -------------------------------
drivers::SysClockDriver sys_clock_driver;

// ------------------------------ DEBUG UART ------------------------------
utils::RingBuffer<128>           debug_uart_tx_buffer;
utils::RingBuffer<128>           debug_uart_rx_buffer;
drivers::BufferedAsyncUartDriver debug_uart(hw_mappings::K_DEBUG_UART_INSTANCE, &debug_uart_tx_buffer,
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
parameter_system::ParameterDelegateBase* parameter_buffer[64] = {nullptr};
parameter_system::ParameterDatabase      parameter_database({parameter_buffer});

// ----------------------------- COMM PROTOCOL --------------------------------
serial_communication_framework::OperationCodeHandlerInfo handler_buffer[64] = {};
serial_communication_framework::SlaveHandler protocol_handler({handler_buffer}, debug_uart, sys_clock_driver, 0);

void debugUartPutChar(char c) { debug_uart.transmitByte(c); }
void DebugUartFlush() { debug_uart.flushTx(); }

void initHW() {
    // --------------- INIT UART ---------------
    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(hw_mappings::K_DEBUG_UART_TX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_TX_PIN));
    gpio_set_function(hw_mappings::K_DEBUG_UART_RX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_RX_PIN));

    irq_set_exclusive_handler(UART0_IRQ, debugUartCombinedISR);
    // Enable the UART IRQ in the NVIC
    irq_set_enabled(UART0_IRQ, true);

    debug_uart.init();

    // --------------- INIT LED PWM ---------------
    gpio_set_function(hw_mappings::K_STATUS_LED_RED_PIN, GPIO_FUNC_PWM);
    gpio_set_function(hw_mappings::K_STATUS_LED_GREEN_PIN, GPIO_FUNC_PWM);
    gpio_set_function(hw_mappings::K_STATUS_LED_BLUE_PIN, GPIO_FUNC_PWM);
    // initializes the PWM:s for color control
    led_driver.init();
    led_driver.turnOn();
    led_driver.setBrightness(25);

    // --------------- INIT LED TIMER ---------------
    led_update_timer.configureInMilliseconds(20);  // Lowering this increases the accuracy of led effects
    irq_set_exclusive_handler(led_update_timer.getIrqNumber(), periodicLedUpdateTimerISR);
    irq_set_enabled(led_update_timer.getIrqNumber(), true);
    led_update_timer.start();

    // --------------- INIT COMMUNICATION ---------------
    protocol_handler.init();
}

void initSWLibs() {
    debug_print::connectPutCharAndFlushFunctions(&debugUartPutChar, &DebugUartFlush);
    assert::setAssertionFailedReaction(assert::OnAssertFailReaction::call_assertion_handler_and_break_point);

    protocol_handler.registerHandler(static_cast<uint8_t>(protocol::Commands::ping), protocol_handlers::ping);
    protocol_handler.registerHandler(protocol::requests::GetRegisteredParameterIds::op_code,
                                     protocol_handlers::getParamIds);
    protocol_handler.registerHandler(protocol::requests::GetParameterMetaData::op_code,
                                     protocol_handlers::getParamMetaData);
}

int main() {
    initHW();  // TODO can sw libs be initialized first?
    initSWLibs();

    DEBUG_PRINT("Starting up!\n");

    uint8_t  test_uint8  = 42;
    uint16_t test_uint16 = 1337;
    uint32_t test_uint32 = 123456;
    float    test_float  = 3.1415;
    bool     test_bool   = true;

    parameter_system::ParamUint8  param1(0x2, parameter_system::ReadWriteAccess::read_write, "Test Uint8", test_uint8);
    parameter_system::ParamUint16 param2(0x3, parameter_system::ReadWriteAccess::read_only, "Test Uint16", test_uint16);
    parameter_system::ParamUint32 param3(0x4, parameter_system::ReadWriteAccess::read_write, "Test Uint32",
                                         test_uint32);
    parameter_system::ParamFloat  param4(0x5, parameter_system::ReadWriteAccess::read_only, "Test Float", test_float);
    parameter_system::ParamBoolean param5(0x6, parameter_system::ReadWriteAccess::read_write, "Test Bool", test_bool);

    parameter_database.registerParameter(&param1);
    parameter_database.registerParameter(&param2);
    parameter_database.registerParameter(&param3);
    parameter_database.registerParameter(&param4);
    parameter_database.registerParameter(&param5);

    for (parameter_system::ParameterDelegateBase* parameter : parameter_database.getParameterDelegates()) {
        const parameter_system::ParameterMetaData* meta_data = parameter->getMetaData();
        DEBUG_PRINT("ID: % Name: % RW: % TYPE: % ", meta_data->id, meta_data->name,
                    (uint8_t)meta_data->read_write_access, (uint8_t)meta_data->type);
        switch (meta_data->type) {
            case parameter_system::ParameterType::uint8:
                DEBUG_PRINT("Value: %",
                            parameter_database
                                .getParameterDelegateByIdAs<parameter_system::ParameterType::uint8>(meta_data->id)
                                ->getValue());
                break;
            case parameter_system::ParameterType::uint16:
                DEBUG_PRINT("Value: %",
                            parameter_database
                                .getParameterDelegateByIdAs<parameter_system::ParameterType::uint16>(meta_data->id)
                                ->getValue());
                break;
            case parameter_system::ParameterType::uint32:
                DEBUG_PRINT("Value: %",
                            parameter_database
                                .getParameterDelegateByIdAs<parameter_system::ParameterType::uint32>(meta_data->id)
                                ->getValue());
                break;
            case parameter_system::ParameterType::uint64:
                DEBUG_PRINT("Value: %",
                            parameter_database
                                .getParameterDelegateByIdAs<parameter_system::ParameterType::uint32>(meta_data->id)
                                ->getValue());
                break;
            case parameter_system::ParameterType::int8:
                DEBUG_PRINT(
                    "Value: %",
                    parameter_database.getParameterDelegateByIdAs<parameter_system::ParameterType::int8>(meta_data->id)
                        ->getValue());
                break;
            case parameter_system::ParameterType::int16:
                DEBUG_PRINT("Value: %",
                            parameter_database
                                .getParameterDelegateByIdAs<parameter_system::ParameterType::int16>(meta_data->id)
                                ->getValue());
                break;
            case parameter_system::ParameterType::int32:
                DEBUG_PRINT("Value: %",
                            parameter_database
                                .getParameterDelegateByIdAs<parameter_system::ParameterType::int32>(meta_data->id)
                                ->getValue());
                break;
            case parameter_system::ParameterType::int64:
                DEBUG_PRINT("Value: %",
                            parameter_database
                                .getParameterDelegateByIdAs<parameter_system::ParameterType::int64>(meta_data->id)
                                ->getValue());
                break;
            case parameter_system::ParameterType::floating_point:
                DEBUG_PRINT(
                    "Value: %",
                    parameter_database
                        .getParameterDelegateByIdAs<parameter_system::ParameterType::floating_point>(meta_data->id)
                        ->getValue());
                break;
            case parameter_system::ParameterType::double_float:
                DEBUG_PRINT("Value: %", parameter_database
                                            .getParameterDelegateByIdAs<parameter_system::ParameterType::double_float>(
                                                meta_data->id)
                                            ->getValue());
                break;
            case parameter_system::ParameterType::boolean:
                DEBUG_PRINT("Value: %",
                            parameter_database
                                .getParameterDelegateByIdAs<parameter_system::ParameterType::boolean>(meta_data->id)
                                ->getValue());
            default:
                break;
        }
        DEBUG_PRINT("\n");
    }

    /// ************************* MAIN LOOP ************************* ///
    while (true) {
        /*
        while (uart0_controller.getReceivedBytesAvailableAmount() > 0) {
            status_led_controller.flashOverrideColor(led_controller::common_colors::K_ORANGE);
            uart0_controller.transmitByte(uart0_controller.readReceivedByte());
        }*/
        protocol_handler.run();
    }

    return 0;
}
