#include "interrupt_service_routines.h"

#include <hardware/uart.h>

#include "drivers/BufferedAsyncUartDriver.h"
#include "drivers/TimerDriver.h"
#include "led_controller/LedController.h"

extern led_controller::LedController status_led_controller;
extern drivers::TimerDriver          led_update_timer;

// buffer size template arguments need to be updated here if they are modified in main
extern drivers::BufferedAsyncUartDriver<128, 128> debug_uart;

ATTRIBUTE_ISR void periodicLedUpdateTimerISR() {
    status_led_controller.periodicUpdate(led_update_timer.getElapsedMilliseconds());

    // Rearming the timer to start the periodic loop
    // Using restart to reduce the possibility of timer drifting if this isr takes long to complete
    led_update_timer.restart();
}

ATTRIBUTE_ISR void debugUartCombinedISR() {
    // TODO create NVIC driver so that all the interrupts are exposed and no need to do stuff like this?

    // access the UART hardware registers
    uart_hw_t* uart_hw = uart_get_hw(uart0);

    // check RX interrupt (bit 4 in UART_MIS)
    if (uart_hw->mis & UART_UARTMIS_RXMIS_BITS) {
        debug_uart.handleRxInterrupt();
    }

    // check TX interrupt (bit 5 in UART_MIS)
    if (uart_hw->mis & UART_UARTMIS_TXMIS_BITS) {
        debug_uart.handleTxInterrupt();
    }
}
