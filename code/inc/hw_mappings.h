#ifndef HW_MAPPINGS_H
#define HW_MAPPINGS_H

#include <hardware/uart.h>

namespace hw_mappings {

const auto     K_DEBUG_UART_INSTANCE          = uart0;  // Cant be constexpr
constexpr auto K_DEBUG_UART_TX_PIN            = 0;
constexpr auto K_DEBUG_UART_RX_PIN            = 1;

constexpr unsigned int K_STATUS_LED_RED_PIN   = 15;
constexpr unsigned int K_STATUS_LED_GREEN_PIN = 9;
constexpr unsigned int K_STATUS_LED_BLUE_PIN  = 13;

}  // namespace hw_mappings
#endif  // HW_MAPPINGS_H
