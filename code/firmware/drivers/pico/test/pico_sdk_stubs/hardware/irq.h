#ifndef HARDWARE_IRQ_H
#define HARDWARE_IRQ_H
// HOST TEST STUB
#include <cstdint>
#define USBCTRL_IRQ 14
typedef void (*irq_handler_t)(void);
void irq_set_exclusive_handler(unsigned num, irq_handler_t h);
void irq_set_enabled(unsigned num, bool enabled);
bool irq_is_enabled(unsigned num);
#endif
