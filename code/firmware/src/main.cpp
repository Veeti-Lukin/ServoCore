#include <hardware/gpio.h>
#include <hardware/irq.h>
#include <hardware/pwm.h>
#include <hardware/structs/uart.h>
#include <hardware/uart.h>
#include <pico/time.h>

#include <cmath>

#include "assert/assert.h"
#include "debug_print/debug_print.h"
#include "drivers/AnalogRgbLedDriver.h"
#include "drivers/BufferedAsyncUartDriver.h"
#include "drivers/PwmSliceDriver.h"
#include "hw_mappings.h"
#include "led_controller/LedController.h"
#include "led_controller/common_colors.h"
#include "parameter_system/ParameterDatabase.h"
#include "parameter_system/ParameterDelegate.h"
#include "serial_communication_framework/SlaveHandler.h"
#include "serial_communication_framework/packets.h"
#include "serial_communication_framework_op_code_handlers.h"
#include "utils/RingBuffer.h"

#define UART_ID   uart0
#define BAUD_RATE 115200

namespace uart_config = drivers::uart_config;

// ------------------------------ DEBUG UART ------------------------------
utils::RingBuffer<128>           uart_tx_buffer;
utils::RingBuffer<128>           uart_rx_buffer;
drivers::BufferedAsyncUartDriver uart0_controller(hw_mappings::K_DEBUG_UART_INSTANCE, &uart_tx_buffer, &uart_rx_buffer,
                                                  115200, uart_config::DataBits::eight, uart_config::StopBits::one,
                                                  uart_config::Parity::none);

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
repeating_timer               timer;

// ----------------------------- PARAMETER SYSTEM ------------------------------
parameter_system::ParameterDelegateBase* parameter_buffer[64] = {nullptr};
parameter_system::ParameterDatabase      parameter_database({parameter_buffer});

// ----------------------------- COMM PROTOCOL --------------------------------
serial_communication_framework::OperationCodeHandlerInfo handler_buffer[64] = {};
serial_communication_framework::SlaveHandler protocol_handler({handler_buffer}, uart0_controller, 0);

void uart0_putchar(char c) { uart0_controller.transmitByte(c); }
void uart0_flush() { uart0_controller.flushTx(); }

void uart0_isr() {
    // access the UART hardware registers
    uart_hw_t* uart_hw = uart_get_hw(uart0);

    // check RX interrupt (bit 4 in UART_MIS)
    if (uart_hw->mis & UART_UARTMIS_RXMIS_BITS) {
        uart0_controller.handleRxInterrupt();
    }

    // check TX interrupt (bit 5 in UART_MIS)
    if (uart_hw->mis & UART_UARTMIS_TXMIS_BITS) {
        uart0_controller.handleTxInterrupt();
    }
}

bool led_timer_isr(__unused repeating_timer_t*) {
    status_led_controller.periodicUpdate(20);
    return true;
}

void initHW() {
    // --------------- INIT UART ---------------
    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(hw_mappings::K_DEBUG_UART_TX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_TX_PIN));
    gpio_set_function(hw_mappings::K_DEBUG_UART_RX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_RX_PIN));

    irq_set_exclusive_handler(UART0_IRQ, uart0_isr);
    // Enable the UART IRQ in the NVIC
    irq_set_enabled(UART0_IRQ, true);

    uart0_controller.init();

    // --------------- INIT LED PWM ---------------
    gpio_set_function(hw_mappings::K_STATUS_LED_RED_PIN, GPIO_FUNC_PWM);
    gpio_set_function(hw_mappings::K_STATUS_LED_GREEN_PIN, GPIO_FUNC_PWM);
    gpio_set_function(hw_mappings::K_STATUS_LED_BLUE_PIN, GPIO_FUNC_PWM);
    // initializes the PWM:s for color control
    led_driver.init();
    led_driver.turnOn();
    led_driver.setBrightness(25);

    // --------------- INIT LED TIMER ---------------
    add_repeating_timer_ms(-20, led_timer_isr, nullptr, &timer);
}

void initSWLibs() {
    debug_print::connectPutCharAndFlushFunctions(&uart0_putchar, &uart0_flush);
    assert::setAssertionFailedReaction(assert::OnAssertFailReaction::call_assertion_handler_and_break_point);
}

int main() {
    initHW();  // TODO can sw libs be initialized first?
    initSWLibs();

    DEBUG_PRINT("Starting up!\n");

    protocol_handler.registerHandler(1, serial_communication_framework_op_code_handlers::echo);

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
