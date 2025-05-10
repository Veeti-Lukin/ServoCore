#include "interrupt_service_routines.h"

#include <hardware/uart.h>

#include "drivers/BufferedAsyncUartDriver.h"
#include "drivers/TimerDriver.h"
#include "hw_mappings.h"
#include "led_controller/LedController.h"
#include "serial_communication_framework/SlaveHandler.h"

extern led_controller::LedController status_led_controller;
extern drivers::TimerDriver          led_update_timer;

// buffer size template arguments need to be updated here if they are modified in main
extern drivers::BufferedAsyncUartDriver<128, 128> debug_uart_driver;
extern drivers::BufferedAsyncUartDriver<128, 128> communication_uart_driver;

extern serial_communication_framework::SlaveHandler protocol_handler;
extern drivers::TimerDriver                         communication_timeout_timer;

ATTRIBUTE_ISR void periodicLedUpdateTimerISR() {
    status_led_controller.periodicUpdate(led_update_timer.getElapsedMilliseconds());

    // Rearming the timer to start the periodic loop
    // Using restart to reduce the possibility of timer drifting if this isr takes long to complete
    led_update_timer.restart();
}

ATTRIBUTE_ISR void debugUartCombinedISR() {
    // TODO create NVIC driver so that all the interrupts are exposed and no need to do stuff like this?

    // access the UART hardware registers
    uart_hw_t* uart_hw = uart_get_hw(hw_mappings::K_DEBUG_UART_INSTANCE);

    // check RX interrupt (bit 4 in UART_MIS)
    if (uart_hw->mis & UART_UARTMIS_RXMIS_BITS) {
        debug_uart_driver.handleRxInterrupt();
    }

    // check TX interrupt (bit 5 in UART_MIS)
    if (uart_hw->mis & UART_UARTMIS_TXMIS_BITS) {
        debug_uart_driver.handleTxInterrupt();
    }
}

ATTRIBUTE_ISR void serialCommunicationUartCombinedISR() {
    // TODO create NVIC driver so that all the interrupts are exposed and no need to do stuff like this?

    // access the UART hardware registers
    uart_hw_t* uart_hw = uart_get_hw(hw_mappings::K_SERIAL_COMMUNICATION_UART_INSTANCE);

    // check RX interrupt (bit 4 in UART_MIS)
    if (uart_hw->mis & UART_UARTMIS_RXMIS_BITS) {
        communication_uart_driver.handleRxInterrupt();
    }

    // check TX interrupt (bit 5 in UART_MIS)
    if (uart_hw->mis & UART_UARTMIS_TXMIS_BITS) {
        communication_uart_driver.handleTxInterrupt();
    }
}