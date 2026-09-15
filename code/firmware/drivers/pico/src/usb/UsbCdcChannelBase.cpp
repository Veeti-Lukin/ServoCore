#include "drivers/usb/UsbCdcChannelBase.h"

#include <cstring>

namespace drivers::usb {

void UsbCdcChannelBase::bind(UsbDeviceBase& device, uint8_t function_index, uint8_t first_interface) {
    device_            = &device;
    function_index_    = function_index;
    control_interface_ = first_interface;
    data_interface_    = static_cast<uint8_t>(first_interface + 1);
    out_endpoint_      = &device.getEndpoint(function_index, K_ENDPOINT_DATA_OUT);
    in_endpoint_       = &device.getEndpoint(function_index, K_ENDPOINT_DATA_IN);
}

bool UsbCdcChannelBase::isConnected() const {
    return configured_ && dtr_ && device_ != nullptr && !device_->isSuspended() && device_->isVbusPresent();
}

bool UsbCdcChannelBase::ownsInterface(uint8_t interface_number) const {
    return interface_number == control_interface_ || interface_number == data_interface_;
}

int UsbCdcChannelBase::onControlIn(const SetupPacket& setup, std::span<uint8_t> response) {
    if (setup.request == cdc::K_REQUEST_GET_LINE_CODING && response.size() >= sizeof(line_coding_)) {
        memcpy(response.data(), &line_coding_, sizeof(line_coding_));
        return sizeof(line_coding_);
    }
    return -1;
}

bool UsbCdcChannelBase::onControlOut(const SetupPacket& setup, std::span<const uint8_t> data) {
    switch (setup.request) {
        case cdc::K_REQUEST_SET_LINE_CODING:
            if (data.size() >= sizeof(line_coding_)) {
                memcpy(&line_coding_, data.data(), sizeof(line_coding_));
            }
            return true;

        case cdc::K_REQUEST_SET_CONTROL_LINE_STATE: {
            const bool previous_dtr = dtr_;
            dtr_                    = (setup.value & cdc::K_CONTROL_LINE_DTR) != 0;
            rts_                    = (setup.value & cdc::K_CONTROL_LINE_RTS) != 0;

            // Bootloader "touch": a host that set 1200 baud and then drops DTR is asking us to reboot into the
            // ROM bootloader so it can flash new firmware without anyone pressing BOOTSEL.
            if (previous_dtr && !dtr_ && device_->isBootloaderTouchEnabled() &&
                line_coding_.baud_rate == cdc::K_BOOTLOADER_TOUCH_BAUD_RATE) {
                device_->rebootToBootloader();
            }
            return true;
        }

        case cdc::K_REQUEST_SEND_BREAK:
            return true;  // accepted, nothing to do on a virtual port

        default:
            return false;
    }
}

void UsbCdcChannelBase::onConfigured() {
    configured_ = true;
    resetLink();
}

void UsbCdcChannelBase::onDisconnected() {
    configured_ = false;
    dtr_        = false;
    rts_        = false;
    resetLink();
}

}  // namespace drivers::usb
