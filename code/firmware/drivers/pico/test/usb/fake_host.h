#ifndef FAKEHOST_H
#define FAKEHOST_H
// Test helper: plays the role of the USB host against the simulated controller. It writes SETUP packets,
// acknowledges IN and OUT buffers the way the controller would (clearing AVAIL/FULL, raising buffer status)
// and raises bus events, then calls the device's interrupt handler.
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "drivers/usb/UsbDeviceBase.h"
#include "drivers/usb/usb_protocol.h"
#include "hardware/structs/usb.h"
#include "hardware/structs/usb_dpram.h"

class FakeHost {
public:
    explicit FakeHost(drivers::usb::UsbDeviceBase& device) : device_(device) {}

    void fireInterrupt(uint32_t ints) {
        usb_hw->ints = ints;
        device_.handleInterrupt();
        usb_hw->ints = 0;
    }

    void busReset() { fireInterrupt(USB_INTS_BUS_RESET_BITS); }
    void suspend() { fireInterrupt(USB_INTS_DEV_SUSPEND_BITS); }
    void resume() { fireInterrupt(USB_INTS_DEV_RESUME_FROM_HOST_BITS); }

    void sendSetup(uint8_t request_type, uint8_t request, uint16_t value, uint16_t index, uint16_t length) {
        const uint8_t packet[8] = {request_type, request,   lo(value),  hi(value),
                                   lo(index),    hi(index), lo(length), hi(length)};
        memcpy(const_cast<uint8_t*>(usb_dpram->setup_packet), packet, 8);
        // A new SETUP clears any previous stall condition on EP0.
        usb_dpram->ep_buf_ctrl[0].in &= ~USB_BUF_CTRL_STALL;
        usb_dpram->ep_buf_ctrl[0].out &= ~USB_BUF_CTRL_STALL;
        usb_hw->ep_stall_arm = 0;
        fireInterrupt(USB_INTS_SETUP_REQ_BITS);
    }

    /// Data PIDs (true = DATA1) of every packet collected so far, per IN endpoint number (0 = control).
    std::vector<bool> in_pids[16];
    /// Data PID the currently armed OUT buffer expects.
    static bool outExpectsData1(uint8_t n) { return (usb_dpram->ep_buf_ctrl[n].out & USB_BUF_CTRL_DATA1_PID) != 0; }

    /// Collect the pending EP0 IN packet and acknowledge it. Returns its payload.
    std::vector<uint8_t> collectControlIn(bool* was_data1 = nullptr) {
        const uint32_t control = usb_dpram->ep_buf_ctrl[0].in;
        const uint16_t length  = control & USB_BUF_CTRL_LEN_MASK;
        in_pids[0].push_back((control & USB_BUF_CTRL_DATA1_PID) != 0);
        if (was_data1) *was_data1 = (control & USB_BUF_CTRL_DATA1_PID) != 0;
        std::vector<uint8_t> payload(usb_dpram->ep0_buf_a, usb_dpram->ep0_buf_a + length);
        usb_dpram->ep_buf_ctrl[0].in &= ~(USB_BUF_CTRL_AVAIL | USB_BUF_CTRL_FULL);
        usb_hw->buf_status = 1u << 0;
        fireInterrupt(USB_INTS_BUFF_STATUS_BITS);
        return payload;
    }

    /// Deliver an EP0 OUT packet (data stage or zero-length status) into the armed buffer.
    void sendControlOut(const uint8_t* data, uint16_t length) {
        if (data && length) memcpy(usb_dpram->ep0_buf_a, data, length);
        usb_dpram->ep_buf_ctrl[0].out =
            (usb_dpram->ep_buf_ctrl[0].out & ~(USB_BUF_CTRL_LEN_MASK | USB_BUF_CTRL_AVAIL)) | length;
        usb_hw->buf_status = 1u << 1;
        fireInterrupt(USB_INTS_BUFF_STATUS_BITS);
    }

    bool controlIsStalled() const { return (usb_dpram->ep_buf_ctrl[0].in & USB_BUF_CTRL_STALL) != 0; }
    bool controlOutArmed() const { return (usb_dpram->ep_buf_ctrl[0].out & USB_BUF_CTRL_AVAIL) != 0; }
    bool controlInArmed() const { return (usb_dpram->ep_buf_ctrl[0].in & USB_BUF_CTRL_AVAIL) != 0; }

    /// Full device-to-host control transfer. Returns the concatenated data stage; records packet sizes.
    std::vector<uint8_t> controlTransferIn(uint8_t request_type, uint8_t request, uint16_t value, uint16_t index,
                                           uint16_t length, std::vector<int>* packet_sizes = nullptr) {
        sendSetup(request_type, request, value, index, length);
        std::vector<uint8_t> all;
        for (int guard = 0; guard < 40; guard++) {
            if (controlIsStalled() || controlOutArmed() || !controlInArmed()) break;
            std::vector<uint8_t> packet = collectControlIn();
            if (packet_sizes) packet_sizes->push_back(static_cast<int>(packet.size()));
            all.insert(all.end(), packet.begin(), packet.end());
        }
        if (controlOutArmed()) sendControlOut(nullptr, 0);  // status stage
        return all;
    }

    /// Host-to-device control transfer with an optional data stage; acknowledges the status stage.
    void controlTransferOut(uint8_t request_type, uint8_t request, uint16_t value, uint16_t index,
                            const uint8_t* data = nullptr, uint16_t length = 0) {
        sendSetup(request_type, request, value, index, length);
        if (length && controlOutArmed()) sendControlOut(data, length);
        if (controlInArmed()) collectControlIn();
    }

    /// ---- non-control endpoints ----
    static uint16_t bufferOffset(uint8_t endpoint_number, bool is_in) {
        return (is_in ? usb_dpram->ep_ctrl[endpoint_number - 1].in : usb_dpram->ep_ctrl[endpoint_number - 1].out) &
               0xFFFF;
    }
    static bool inArmed(uint8_t n) { return (usb_dpram->ep_buf_ctrl[n].in & USB_BUF_CTRL_AVAIL) != 0; }
    static bool outArmed(uint8_t n) { return (usb_dpram->ep_buf_ctrl[n].out & USB_BUF_CTRL_AVAIL) != 0; }

    /// Collect the packet the device offered on IN endpoint n and acknowledge it.
    std::vector<uint8_t> collectIn(uint8_t n) {
        const uint16_t length = usb_dpram->ep_buf_ctrl[n].in & USB_BUF_CTRL_LEN_MASK;
        in_pids[n].push_back((usb_dpram->ep_buf_ctrl[n].in & USB_BUF_CTRL_DATA1_PID) != 0);
        const uint8_t*       buffer = reinterpret_cast<const uint8_t*>(usb_dpram) + bufferOffset(n, true);
        std::vector<uint8_t> payload(buffer, buffer + length);
        usb_dpram->ep_buf_ctrl[n].in &= ~(USB_BUF_CTRL_AVAIL | USB_BUF_CTRL_FULL);
        usb_hw->buf_status = 1u << (n * 2);
        fireInterrupt(USB_INTS_BUFF_STATUS_BITS);
        return payload;
    }

    /// Deliver a packet into the armed OUT endpoint n.
    void sendOut(uint8_t n, const uint8_t* data, uint16_t length) {
        uint8_t* buffer = reinterpret_cast<uint8_t*>(usb_dpram) + bufferOffset(n, false);
        memcpy(buffer, data, length);
        usb_dpram->ep_buf_ctrl[n].out =
            (usb_dpram->ep_buf_ctrl[n].out & ~(USB_BUF_CTRL_LEN_MASK | USB_BUF_CTRL_AVAIL)) | length;
        usb_hw->buf_status = 1u << (n * 2 + 1);
        fireInterrupt(USB_INTS_BUFF_STATUS_BITS);
    }
    void sendOut(uint8_t n, const std::string& s) {
        sendOut(n, reinterpret_cast<const uint8_t*>(s.data()), static_cast<uint16_t>(s.size()));
    }

    /// ---- standard enumeration shortcuts ----
    static constexpr uint8_t K_IN_DEVICE     = 0x80;
    static constexpr uint8_t K_OUT_DEVICE    = 0x00;
    static constexpr uint8_t K_IN_CLASS_ITF  = 0xA1;
    static constexpr uint8_t K_OUT_CLASS_ITF = 0x21;

    std::vector<uint8_t> getDescriptor(uint8_t type, uint8_t index, uint16_t length, uint16_t lang = 0,
                                       std::vector<int>* sizes = nullptr) {
        return controlTransferIn(K_IN_DEVICE, drivers::usb::K_REQUEST_GET_DESCRIPTOR,
                                 static_cast<uint16_t>((type << 8) | index), lang, length, sizes);
    }

    void setAddress(uint8_t address) {
        controlTransferOut(K_OUT_DEVICE, drivers::usb::K_REQUEST_SET_ADDRESS, address, 0);
    }
    void setConfiguration(uint8_t value = 1) {
        controlTransferOut(K_OUT_DEVICE, drivers::usb::K_REQUEST_SET_CONFIGURATION, value, 0);
    }

    /// Everything a real host does before a port is usable.
    void enumerate() {
        getDescriptor(drivers::usb::K_DESCRIPTOR_TYPE_DEVICE, 0, 64);
        busReset();
        setAddress(5);
        getDescriptor(drivers::usb::K_DESCRIPTOR_TYPE_DEVICE, 0, 18);
        getDescriptor(drivers::usb::K_DESCRIPTOR_TYPE_CONFIGURATION, 0, 255);
        setConfiguration(1);
    }

    /// CDC helpers. `control_interface` is the function's communication interface number.
    void setLineCoding(uint8_t control_interface, uint32_t baud, uint8_t data_bits = 8) {
        drivers::usb::cdc::LineCoding lc{baud, 0, 0, data_bits};
        controlTransferOut(K_OUT_CLASS_ITF, drivers::usb::cdc::K_REQUEST_SET_LINE_CODING, 0, control_interface,
                           reinterpret_cast<const uint8_t*>(&lc), sizeof(lc));
    }
    void setControlLineState(uint8_t control_interface, bool dtr, bool rts = true) {
        const uint16_t value = static_cast<uint16_t>((dtr ? 1 : 0) | (rts ? 2 : 0));
        controlTransferOut(K_OUT_CLASS_ITF, drivers::usb::cdc::K_REQUEST_SET_CONTROL_LINE_STATE, value,
                           control_interface);
    }
    /// Open the port the way a terminal or QSerialPort does: line coding, then DTR+RTS.
    void openPort(uint8_t control_interface, uint32_t baud = 115200) {
        setLineCoding(control_interface, baud);
        setControlLineState(control_interface, true, true);
    }

private:
    static uint8_t lo(uint16_t v) { return static_cast<uint8_t>(v & 0xFF); }
    static uint8_t hi(uint16_t v) { return static_cast<uint8_t>(v >> 8); }

    drivers::usb::UsbDeviceBase& device_;
};

#endif  // FAKEHOST_H
