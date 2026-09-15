#ifndef HARDWARE_ADDRESS_MAPPED_H
#define HARDWARE_ADDRESS_MAPPED_H
// HOST TEST STUB. On real silicon the set/clear aliases are address windows that OR / AND-NOT the written
// value into the register. Here they are separate shadow structs whose element types implement that in operator=.
#include <cstdint>
typedef volatile uint32_t io_rw_32;
typedef volatile uint32_t io_ro_32;
typedef volatile uint32_t io_wo_32;
void* hw_set_alias_untyped(volatile void* addr);
void* hw_clear_alias_untyped(volatile void* addr);
static inline void hw_set_bits(io_rw_32* r, uint32_t m)   { *r |= m; }
static inline void hw_clear_bits(io_rw_32* r, uint32_t m) { *r &= ~m; }
#endif
