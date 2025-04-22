#ifndef FIRMWARE_INTERRUPT_SERVICE_ROUTINES_H
#define FIRMWARE_INTERRUPT_SERVICE_ROUTINES_H

/**
 * Informs GCC that the function is an interrupt handler.
 * Applies a special calling convention:
 *      Saves additional registers (r8–r12, lr, etc.).
 *      Modifies function prologue/epilogue for safe ISR usage.
 *      Ensures correct bx lr return from ISR.
 */
#define ATTRIBUTE_ISR __attribute__((interrupt("IRQ")))

// This isr can be extended to host other periodic general updates too
ATTRIBUTE_ISR void periodicLedUpdateTimerISR();

#endif  // FIRMWARE_INTERRUPT_SERVICE_ROUTINES_H
