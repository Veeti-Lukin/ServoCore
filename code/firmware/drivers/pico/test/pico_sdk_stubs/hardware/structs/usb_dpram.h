#ifndef HARDWARE_STRUCTS_USB_DPRAM_H
#define HARDWARE_STRUCTS_USB_DPRAM_H
// HOST TEST STUB. Layout and macros follow pico-sdk hardware/structs/usb_dpram.h exactly.
#include <cstddef>
#include <cstdint>
#include "hardware/address_mapped.h"
#include "hardware/regs/usb.h"

#define USB_NUM_ENDPOINTS 16
#define USB_DPRAM_SIZE 4096u
#define USB_BUF_CTRL_FULL      0x00008000u
#define USB_BUF_CTRL_LAST      0x00004000u
#define USB_BUF_CTRL_DATA0_PID 0x00000000u
#define USB_BUF_CTRL_DATA1_PID 0x00002000u
#define USB_BUF_CTRL_SEL       0x00001000u
#define USB_BUF_CTRL_STALL     0x00000800u
#define USB_BUF_CTRL_AVAIL     0x00000400u
#define USB_BUF_CTRL_LEN_MASK  0x000003FFu
#define EP_CTRL_ENABLE_BITS (1u << 31u)
#define EP_CTRL_DOUBLE_BUFFERED_BITS (1u << 30)
#define EP_CTRL_INTERRUPT_PER_BUFFER (1u << 29)
#define EP_CTRL_BUFFER_TYPE_LSB 26u
#define USB_DPRAM_MAX USB_DPRAM_SIZE

typedef struct {
    volatile uint8_t setup_packet[8];
    struct { io_rw_32 in; io_rw_32 out; } ep_ctrl[USB_NUM_ENDPOINTS - 1];
    struct { io_rw_32 in; io_rw_32 out; } ep_buf_ctrl[USB_NUM_ENDPOINTS];
    uint8_t ep0_buf_a[0x40];
    uint8_t ep0_buf_b[0x40];
    uint8_t epx_data[USB_DPRAM_MAX - 0x180];
} usb_device_dpram_t;
static_assert(offsetof(usb_device_dpram_t, epx_data) == 0x180, "");
static_assert(sizeof(usb_device_dpram_t) == USB_DPRAM_SIZE, "");

extern usb_device_dpram_t* const usb_dpram;
#endif
