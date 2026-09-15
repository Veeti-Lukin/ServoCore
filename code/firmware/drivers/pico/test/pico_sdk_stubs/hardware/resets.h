#ifndef HARDWARE_RESETS_H
#define HARDWARE_RESETS_H
// HOST TEST STUB
#include <cstdint>
#define RESETS_RESET_USBCTRL_BITS (1u << 28)
void reset_block_mask(uint32_t bits);
void unreset_block_mask_wait_blocking(uint32_t bits);
#endif
