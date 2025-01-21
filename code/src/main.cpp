#include <hardware/gpio.h>
#include <hardware/irq.h>
#include <hardware/structs/uart.h>
#include <hardware/uart.h>

#include "drivers/BufferedAsyncUartDriver.h"
#include "utils/RingBuffer.h"

#define UART_ID   uart0
#define BAUD_RATE 115200

namespace uart_config = drivers::uart_config;

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_TX_PIN 0
#define UART_RX_PIN 1

utils::RingBuffer<128>           uart_tx_buffer;
utils::RingBuffer<128>           uart_rx_buffer;
drivers::BufferedAsyncUartDriver uart0_controller(uart0, &uart_tx_buffer, &uart_rx_buffer, 115200,
                                                  uart_config::DataBits::eight, uart_config::StopBits::one,
                                                  uart_config::Parity::none);

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

    // clear handled interrupts (mandatory)
    uart_hw->icr = UART_UARTICR_RXIC_BITS | UART_UARTICR_TXIC_BITS;
}

void initHW() {
    // ----- INIT UART -----
    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_TX_PIN));
    gpio_set_function(UART_RX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_RX_PIN));

    irq_set_exclusive_handler(UART0_IRQ, uart0_isr);
    // Enable the UART IRQ in the NVIC
    irq_set_enabled(UART0_IRQ, true);

    uart0_controller.init();
}

int main() {
    initHW();

    uart0_controller.transmitString("Hello World!\r\n");

    while (true) {
        while (uart0_controller.getReceivedBytesAvailableAmount() > 0) {
            uart0_controller.transmitByte(uart0_controller.readReceivedByte());
        }
    }
    return 0;
}
