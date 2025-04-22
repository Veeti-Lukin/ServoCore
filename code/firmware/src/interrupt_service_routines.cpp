#include "interrupt_service_routines.h"

#include <cstdint>

#include "drivers/TimerDriver.h"
#include "led_controller/LedController.h"

extern led_controller::LedController status_led_controller;

extern drivers::TimerDriver led_update_timer;

ATTRIBUTE_ISR void periodicLedUpdateTimerISR() {
    status_led_controller.periodicUpdate(led_update_timer.getElapsedMilliseconds());

    // Rearming the timer to start the periodic loop
    // Using restart to reduce the possibility of timer drifting if this isr takes long to complete
    led_update_timer.restart();
}