#ifndef HW_MAPPINGS_H
#define HW_MAPPINGS_H

#include <drivers/TimerDriver.h>
#include <hardware/timer.h>
#include <hardware/uart.h>

namespace hw_mappings {

const auto     K_SERIAL_COMMUNICATION_UART_INSTANCE = uart0;
constexpr auto K_SERIAL_COMMUNICATION_UART_TX_PIN   = 0;
constexpr auto K_SERIAL_COMMUNICATION_UART_RX_PIN   = 1;

const auto     K_DEBUG_UART_INSTANCE                = uart1;  // Cant be constexpr
constexpr auto K_DEBUG_UART_TX_PIN                  = 4;
constexpr auto K_DEBUG_UART_RX_PIN                  = 5;

constexpr unsigned int K_STATUS_LED_RED_PIN         = 15;
constexpr unsigned int K_STATUS_LED_GREEN_PIN       = 9;
constexpr unsigned int K_STATUS_LED_BLUE_PIN        = 13;

const auto     K_PERIODIC_LED_TIMER_INSTANCE        = timer_hw;
constexpr auto K_PERIODIC_LED_TIMER_ALARM_CHANNEL   = drivers::TimerAlarmChannel::alarm3;

}  // namespace hw_mappings

#endif  // HW_MAPPINGS_H
