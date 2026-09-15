#ifndef HARDWARE_STRUCTS_USB_H
#define HARDWARE_STRUCTS_USB_H
// HOST TEST STUB. Register names and order follow pico-sdk hardware/structs/usb.h (RP2350).
//
// On silicon, hw_set_alias_untyped(usb_hw) / hw_clear_alias_untyped(usb_hw) are address windows: a plain store
// through a usb_hw_t* at those addresses ORs / AND-NOTs into the real register. To keep the driver's
// reinterpret_cast<usb_hw_t*>(alias) idiom working on the host, each register is a small class whose operator=
// looks at where `this` lives (plain block, set window or clear window) and applies the right semantics.
#include <cstdint>
#include "hardware/address_mapped.h"
#include "hardware/regs/usb.h"

struct usb_stub_reg {
    uint32_t      value;
    operator uint32_t() const { return value; }
    usb_stub_reg& operator=(uint32_t v);
    usb_stub_reg& operator|=(uint32_t v) { return *this = (value | v); }
    usb_stub_reg& operator&=(uint32_t v) { return *this = (value & v); }
};

typedef struct {
    usb_stub_reg dev_addr_ctrl;
    usb_stub_reg int_ep_addr_ctrl[15];
    usb_stub_reg main_ctrl;
    usb_stub_reg sof_wr;
    usb_stub_reg sof_rd;
    usb_stub_reg sie_ctrl;
    usb_stub_reg sie_status;
    usb_stub_reg int_ep_ctrl;
    usb_stub_reg buf_status;
    usb_stub_reg buf_cpu_should_handle;
    usb_stub_reg abort;
    usb_stub_reg abort_done;
    usb_stub_reg ep_stall_arm;
    usb_stub_reg nak_poll;
    usb_stub_reg ep_nak_stall_status;
    usb_stub_reg muxing;
    usb_stub_reg pwr;
    usb_stub_reg phy_direct;
    usb_stub_reg phy_direct_override;
    usb_stub_reg phy_trim;
    usb_stub_reg linestate_tuning;
    usb_stub_reg intr;
    usb_stub_reg inte;
    usb_stub_reg intf;
    usb_stub_reg ints;
} usb_hw_t;

extern usb_hw_t* const usb_hw;
#endif
