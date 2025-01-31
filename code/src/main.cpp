#include <hardware/gpio.h>
#include <hardware/irq.h>
#include <hardware/pwm.h>
#include <hardware/structs/uart.h>
#include <hardware/uart.h>
#include <pico/time.h>

#include <cmath>

#include "debug_print/debug_print.h"
#include "drivers/AnalogRgbLedDriver.h"
#include "drivers/BufferedAsyncUartDriver.h"
#include "drivers/PwmSliceDriver.h"
#include "utils/RingBuffer.h"

#define UART_ID   uart0
#define BAUD_RATE 115200

namespace uart_config = drivers::uart_config;

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_TX_PIN 0
#define UART_RX_PIN 1

#define RGB_LED_RED_PIN   15
#define RGB_LED_GREEN_PIN 9
#define RGB_LED_BLUE_PIN  13

// ------------------------------ DEBUG UART ------------------------------
utils::RingBuffer<128>           uart_tx_buffer;
utils::RingBuffer<128>           uart_rx_buffer;
drivers::BufferedAsyncUartDriver uart0_controller(uart0, &uart_tx_buffer, &uart_rx_buffer, 115200,
                                                  uart_config::DataBits::eight, uart_config::StopBits::one,
                                                  uart_config::Parity::none);

// --------------------------------- LED ---------------------------------
drivers::PwmSliceDriver     red_slice_driver(pwm_gpio_to_slice_num(RGB_LED_RED_PIN));
drivers::PwmSliceDriver     green_slice_driver(pwm_gpio_to_slice_num(RGB_LED_GREEN_PIN));
drivers::PwmSliceDriver     blue_slice_driver(pwm_gpio_to_slice_num(RGB_LED_BLUE_PIN));
drivers::AnalogRgbLedDriver led_driver(drivers::AnalogRgbLedType::common_cathode, &red_slice_driver,
                                       drivers::mapIndexToPwmChannel(pwm_gpio_to_channel(RGB_LED_RED_PIN)),
                                       &green_slice_driver,
                                       drivers::mapIndexToPwmChannel(pwm_gpio_to_channel(RGB_LED_GREEN_PIN)),
                                       &blue_slice_driver,
                                       drivers::mapIndexToPwmChannel(pwm_gpio_to_channel(RGB_LED_BLUE_PIN)));

void uart0_putchar(char c) { uart0_controller.transmitByte(c); }

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

void initHW() {
    // --------------- INIT UART ---------------
    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_TX_PIN));
    gpio_set_function(UART_RX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_RX_PIN));

    irq_set_exclusive_handler(UART0_IRQ, uart0_isr);
    // Enable the UART IRQ in the NVIC
    irq_set_enabled(UART0_IRQ, true);

    uart0_controller.init();

    // --------------- INIT LED PWM ---------------
    gpio_set_function(RGB_LED_RED_PIN, GPIO_FUNC_PWM);
    gpio_set_function(RGB_LED_GREEN_PIN, GPIO_FUNC_PWM);
    gpio_set_function(RGB_LED_BLUE_PIN, GPIO_FUNC_PWM);
    // initializes the PWM:s for color control
    led_driver.init();
    led_driver.turnOn();
}

void initSWLibs() { debug_print::connectPutCharFunction(&uart0_putchar); }

int main() {
    initHW();
    initSWLibs();

    DEBUG_PRINT("Starting up!\n");

    led_driver.setBrightness(15);
    led_driver.setColorRGB(255, 0, 0);
    sleep_ms(1000);
    led_driver.setColorRGB(0, 255, 0);
    sleep_ms(1000);
    led_driver.setColorRGB(0, 0, 255);
    sleep_ms(1000);
    led_driver.setColorRGB(255, 255, 255);

    while (true) {
        while (uart0_controller.getReceivedBytesAvailableAmount() > 0) {
            uart0_controller.transmitByte(uart0_controller.readReceivedByte());
        }
    }

    return 0;
}
