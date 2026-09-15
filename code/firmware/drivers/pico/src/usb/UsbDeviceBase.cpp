#include "drivers/usb/UsbDeviceBase.h"

#include <hardware/address_mapped.h>
#include <hardware/gpio.h>
#include <hardware/regs/usb.h>
#include <hardware/resets.h>
#include <hardware/structs/usb.h>
#include <hardware/structs/usb_dpram.h>
#include <pico/bootrom.h>
#include <pico/unique_id.h>

#include <cstring>

#include "assert/assert.h"

namespace drivers::usb {

namespace {

// Atomic set / clear aliases of the controller register block. The SDK does not define these for USB; its own
// low-level example defines them the same way.
#define USB_HW_SET   (reinterpret_cast<usb_hw_t*>(hw_set_alias_untyped(usb_hw)))
#define USB_HW_CLEAR (reinterpret_cast<usb_hw_t*>(hw_clear_alias_untyped(usb_hw)))

// Endpoint-control register fields (DPRAM ep_ctrl).
constexpr uint32_t K_EP_CTRL_ENABLE         = EP_CTRL_ENABLE_BITS;
constexpr uint32_t K_EP_CTRL_INT_PER_BUFFER = EP_CTRL_INTERRUPT_PER_BUFFER;
constexpr uint32_t K_EP_CTRL_TYPE_LSB       = EP_CTRL_BUFFER_TYPE_LSB;

// Buffer-status bits: IN of endpoint n is bit 2n, OUT of endpoint n is bit 2n+1.
constexpr uint32_t bufferStatusBit(uint8_t endpoint_number, bool is_in) {
    return 1u << (endpoint_number * 2 + (is_in ? 0 : 1));
}

constexpr uint32_t K_EP0_IN_STATUS_BIT  = bufferStatusBit(0, true);
constexpr uint32_t K_EP0_OUT_STATUS_BIT = bufferStatusBit(0, false);

uint8_t* dpramBase() { return reinterpret_cast<uint8_t*>(usb_dpram); }

}  // namespace

/// ------------------------------ Construction / lifecycle ------------------------------

UsbDeviceBase::UsbDeviceBase(int vbus_detect_pin)
    : vbus_detect_pin_(vbus_detect_pin), vbus_sense_(vbusSenseModeForPin(vbus_detect_pin)) {}

UsbDeviceBase::VbusSense UsbDeviceBase::vbusSenseModeForPin(int pin) {
    if (pin < 0) {
        return VbusSense::none;
    }
    // The controller's VBUS_DETECT input is selectable on every third GPIO (see IO_BANK0 FUNCSEL tables).
    return (pin % 3 == 1) ? VbusSense::controller_pin : VbusSense::polled_gpio;
}

void UsbDeviceBase::attachTables(const Tables& tables) {
    tables_ = tables;
    ASSERT_WITH_MESSAGE(tables_.endpoint_plans.size() == tables_.endpoint_states.size(),
                        "Endpoint plan and state tables must have the same length");
    // Endpoint states are looked up by functions when they bind, before init() programs the hardware.
    for (size_t i = 0; i < tables_.endpoint_plans.size(); i++) {
        tables_.endpoint_states[i].plan = &tables_.endpoint_plans[i];
    }
}

void UsbDeviceBase::init() {
    // Serial number from the chip's unique ID unless the application supplied one.
    if (tables_.strings.size() > 3 && tables_.strings[3] == nullptr) {
        pico_get_unique_board_id_string(unique_serial_, sizeof(unique_serial_));
        tables_.strings[3] = unique_serial_;
    }

    configureVbusSense();
    resetController();
    programEndpoints();

    switch (vbus_sense_) {
        case VbusSense::none:
            // Nothing to sense: connect unconditionally.
            stats_.vbus_present = true;
            connect();
            break;
        case VbusSense::controller_pin:
            // The pull-up is requested now and the controller applies it only while its VBUS_DETECT input is high.
            stats_.vbus_present = (usb_hw->sie_status & USB_SIE_STATUS_VBUS_DETECTED_BITS) != 0;
            connect();
            break;
        case VbusSense::polled_gpio:
            stats_.vbus_present = gpio_get(static_cast<uint>(vbus_detect_pin_));
            if (stats_.vbus_present) {
                connect();
            }
            break;
    }
}

void UsbDeviceBase::deInit() {
    disconnect();
    irq_set_enabled(K_NVIC_INTERRUPT_NUMBER, false);
    reset_block_mask(RESETS_RESET_USBCTRL_BITS);
}

void UsbDeviceBase::run() {
    switch (vbus_sense_) {
        case VbusSense::none:
            return;
        case VbusSense::controller_pin:
            // Status only: the controller has already applied or released the pull-up itself.
            onVbusChanged((usb_hw->sie_status & USB_SIE_STATUS_VBUS_DETECTED_BITS) != 0);
            return;
        case VbusSense::polled_gpio:
            onVbusChanged(gpio_get(static_cast<uint>(vbus_detect_pin_)));
            return;
    }
}

void UsbDeviceBase::onVbusChanged(bool present) {
    if (present == stats_.vbus_present) {
        return;
    }
    stats_.vbus_present = present;
    if (vbus_sense_ == VbusSense::polled_gpio) {
        if (present) {
            connect();
        } else {
            disconnect();
        }
    } else if (!present) {
        // Cable pulled: the controller dropped the pull-up; drop the address and the session on our side too.
        usb_hw->dev_addr_ctrl = 0;
        resetTransferState();
    }
}

/// ------------------------------ Hardware ------------------------------

void UsbDeviceBase::resetController() {
    reset_block_mask(RESETS_RESET_USBCTRL_BITS);
    unreset_block_mask_wait_blocking(RESETS_RESET_USBCTRL_BITS);

    memset(dpramBase(), 0, K_DPRAM_SIZE);

    // Route the controller to the on-chip PHY, under software connect control.
    usb_hw->muxing = USB_USB_MUXING_TO_PHY_BITS | USB_USB_MUXING_SOFTCON_BITS;
    // VBUS detect: from the muxed pin when the board has one the controller can read; otherwise forced on, and in
    // the polled mode the pull-up is gated by run() instead.
    if (vbus_sense_ == VbusSense::controller_pin) {
        usb_hw->pwr = 0;
    } else {
        usb_hw->pwr = USB_USB_PWR_VBUS_DETECT_BITS | USB_USB_PWR_VBUS_DETECT_OVERRIDE_EN_BITS;
    }
    // Device mode.
    usb_hw->main_ctrl = USB_MAIN_CTRL_CONTROLLER_EN_BITS;
    // One interrupt per endpoint-0 buffer; the pull-up is left off until connect().
    usb_hw->sie_ctrl  = USB_SIE_CTRL_EP0_INT_1BUF_BITS;
    usb_hw->inte      = USB_INTS_BUFF_STATUS_BITS | USB_INTS_BUS_RESET_BITS | USB_INTS_SETUP_REQ_BITS |
                        USB_INTS_DEV_SUSPEND_BITS | USB_INTS_DEV_RESUME_FROM_HOST_BITS;

    resetTransferState();
}

void UsbDeviceBase::configureVbusSense() {
    switch (vbus_sense_) {
        case VbusSense::none:
            break;
        case VbusSense::controller_pin:
            // Hand the pin to the USB controller; keep a pull-down so an unpopulated divider reads "no VBUS".
            gpio_set_function(static_cast<uint>(vbus_detect_pin_), GPIO_FUNC_USB);
            gpio_pull_down(static_cast<uint>(vbus_detect_pin_));
            break;
        case VbusSense::polled_gpio:
            gpio_init(static_cast<uint>(vbus_detect_pin_));
            gpio_set_dir(static_cast<uint>(vbus_detect_pin_), false);
            gpio_pull_down(static_cast<uint>(vbus_detect_pin_));
            break;
    }
}

void UsbDeviceBase::programEndpoints() {
    for (size_t i = 0; i < tables_.endpoint_plans.size(); i++) {
        const EndpointPlan& plan  = tables_.endpoint_plans[i];
        EndpointState&      state = tables_.endpoint_states[i];
        const uint8_t       n     = plan.number();

        ASSERT_WITH_MESSAGE(n >= 1 && n <= K_MAX_ENDPOINTS_PER_DIRECTION, "Endpoint number out of range");

        state.plan                 = &plan;
        state.endpoint_control     = plan.isIn() ? &usb_dpram->ep_ctrl[n - 1].in : &usb_dpram->ep_ctrl[n - 1].out;
        state.buffer_control       = plan.isIn() ? &usb_dpram->ep_buf_ctrl[n].in : &usb_dpram->ep_buf_ctrl[n].out;
        state.buffer               = dpramBase() + plan.dpram_offset;
        state.next_packet_is_data1 = false;
        state.busy                 = false;

        *state.endpoint_control = K_EP_CTRL_ENABLE | K_EP_CTRL_INT_PER_BUFFER |
                                  (static_cast<uint32_t>(plan.transfer_type) << K_EP_CTRL_TYPE_LSB) | plan.dpram_offset;
    }
}

void UsbDeviceBase::connect() {
    if (pullup_enabled_) {
        return;
    }
    pullup_enabled_      = true;
    USB_HW_SET->sie_ctrl = USB_SIE_CTRL_PULLUP_EN_BITS;
}

void UsbDeviceBase::disconnect() {
    if (pullup_enabled_) {
        USB_HW_CLEAR->sie_ctrl = USB_SIE_CTRL_PULLUP_EN_BITS;
        pullup_enabled_        = false;
    }
    usb_hw->dev_addr_ctrl = 0;
    resetTransferState();
}

// The controller can pick up a buffer the moment AVAIL is set, so the length and flags are written first and
// AVAIL is set in a second write after a short delay (this is the sequence the SDK's low-level example uses).
void UsbDeviceBase::writeBufferControl(volatile uint32_t* buffer_control, uint32_t value) {
    *buffer_control = value & ~USB_BUF_CTRL_AVAIL;
    __asm volatile("nop\nnop\nnop");
    *buffer_control = value;
}

/// ------------------------------ Services for functions ------------------------------

UsbDeviceBase::InterruptGuard::InterruptGuard() : was_enabled_(irq_is_enabled(K_NVIC_INTERRUPT_NUMBER)) {
    if (was_enabled_) {
        irq_set_enabled(K_NVIC_INTERRUPT_NUMBER, false);
    }
}

UsbDeviceBase::InterruptGuard::~InterruptGuard() {
    if (was_enabled_) {
        irq_set_enabled(K_NVIC_INTERRUPT_NUMBER, true);
    }
}

EndpointState& UsbDeviceBase::getEndpoint(uint8_t function_index, uint8_t index_in_function) {
    for (size_t i = 0; i < tables_.endpoint_plans.size(); i++) {
        const EndpointPlan& plan = tables_.endpoint_plans[i];
        if (plan.function_index == function_index && plan.index_in_function == index_in_function) {
            return tables_.endpoint_states[i];
        }
    }
    ASSERT_WITH_MESSAGE(false, "Function asked for an endpoint that is not in the plan");
    return tables_.endpoint_states[0];
}

void UsbDeviceBase::armOut(EndpointState& endpoint) {
    uint32_t value = endpoint.plan->max_packet_size | USB_BUF_CTRL_AVAIL;
    if (endpoint.next_packet_is_data1) {
        value |= USB_BUF_CTRL_DATA1_PID;
    }
    endpoint.next_packet_is_data1 = !endpoint.next_packet_is_data1;
    endpoint.busy                 = true;
    writeBufferControl(endpoint.buffer_control, value);
}

void UsbDeviceBase::startIn(EndpointState& endpoint, std::span<const uint8_t> data) {
    ASSERT_WITH_MESSAGE(data.size() <= endpoint.plan->max_packet_size, "IN packet larger than max packet size");
    if (!data.empty()) {
        memcpy(endpoint.buffer, data.data(), data.size());
    }
    uint32_t value = static_cast<uint32_t>(data.size()) | USB_BUF_CTRL_FULL | USB_BUF_CTRL_AVAIL;
    if (endpoint.next_packet_is_data1) {
        value |= USB_BUF_CTRL_DATA1_PID;
    }
    endpoint.next_packet_is_data1 = !endpoint.next_packet_is_data1;
    endpoint.busy                 = true;
    writeBufferControl(endpoint.buffer_control, value);
}

void UsbDeviceBase::rebootToBootloader() { reset_usb_boot(0, 0); }

/// ------------------------------ Interrupt handling ------------------------------

void UsbDeviceBase::handleInterrupt() {
    const uint32_t status = usb_hw->ints;

    if (status & USB_INTS_BUS_RESET_BITS) {
        USB_HW_CLEAR->sie_status = USB_SIE_STATUS_BUS_RESET_BITS;
        onBusReset();
    }

    if (status & USB_INTS_DEV_SUSPEND_BITS) {
        USB_HW_CLEAR->sie_status = USB_SIE_STATUS_SUSPENDED_BITS;
        stats_.suspended         = true;
    }

    if (status & USB_INTS_DEV_RESUME_FROM_HOST_BITS) {
        USB_HW_CLEAR->sie_status = USB_SIE_STATUS_RESUME_BITS;
        stats_.suspended         = false;
    }

    if (status & USB_INTS_SETUP_REQ_BITS) {
        USB_HW_CLEAR->sie_status = USB_SIE_STATUS_SETUP_REC_BITS;
        stats_.suspended         = false;  // any bus traffic means the host is awake
        handleSetupPacket();
    }

    if (status & USB_INTS_BUFF_STATUS_BITS) {
        const uint32_t buffer_status = usb_hw->buf_status;

        if (buffer_status & K_EP0_IN_STATUS_BIT) {
            USB_HW_CLEAR->buf_status = K_EP0_IN_STATUS_BIT;
            onControlInComplete();
        }
        if (buffer_status & K_EP0_OUT_STATUS_BIT) {
            const uint16_t length    = usb_dpram->ep_buf_ctrl[0].out & USB_BUF_CTRL_LEN_MASK;
            USB_HW_CLEAR->buf_status = K_EP0_OUT_STATUS_BIT;
            onControlOutComplete(length);
        }

        for (size_t i = 0; i < tables_.endpoint_plans.size(); i++) {
            const EndpointPlan& plan = tables_.endpoint_plans[i];
            const uint32_t      bit  = bufferStatusBit(plan.number(), plan.isIn());
            if (buffer_status & bit) {
                USB_HW_CLEAR->buf_status = bit;
                EndpointState& state     = tables_.endpoint_states[i];
                const uint16_t length    = *state.buffer_control & USB_BUF_CTRL_LEN_MASK;
                state.busy               = false;
                tables_.functions[plan.function_index]->onEndpointComplete(state, length);
            }
        }
    }
}

void UsbDeviceBase::onBusReset() {
    stats_.bus_resets++;
    usb_hw->dev_addr_ctrl = 0;
    resetTransferState();
}

void UsbDeviceBase::resetTransferState() {
    const bool was_configured     = stats_.configured;
    stats_.configured             = false;
    stats_.suspended              = false;
    address_pending_              = false;
    control_stage_                = ControlStage::idle;
    // Drop anything in flight: the controller does not clear buffer controls on a bus reset by itself, and a
    // packet armed for the old session must never be delivered to the new one.
    usb_dpram->ep_buf_ctrl[0].in  = 0;
    usb_dpram->ep_buf_ctrl[0].out = 0;
    for (EndpointState& state : tables_.endpoint_states) {
        state.busy                 = false;
        state.next_packet_is_data1 = false;
        if (state.buffer_control != nullptr) {
            *state.buffer_control = 0;
        }
    }
    if (was_configured) {
        for (UsbFunctionBase* function : tables_.functions) {
            function->onDisconnected();
        }
    }
}

UsbFunctionBase* UsbDeviceBase::findFunctionForInterface(uint8_t interface_number) {
    for (UsbFunctionBase* function : tables_.functions) {
        if (function->ownsInterface(interface_number)) {
            return function;
        }
    }
    return nullptr;
}

/// ------------------------------ Endpoint 0: control transfers ------------------------------

void UsbDeviceBase::handleSetupPacket() {
    stats_.setup_packets++;
    memcpy(&current_setup_, const_cast<const uint8_t*>(usb_dpram->setup_packet), sizeof(current_setup_));

    // Every control transfer starts its data and status stages with DATA1.
    control_in_data1_  = true;
    control_out_data1_ = true;
    control_stage_     = ControlStage::idle;

    switch (current_setup_.request_type & K_REQUEST_TYPE_MASK) {
        case K_REQUEST_TYPE_STANDARD:
            handleStandardRequest(current_setup_);
            break;
        case K_REQUEST_TYPE_CLASS:
            handleClassRequest(current_setup_);
            break;
        default:
            stallControl();
            break;
    }
}

void UsbDeviceBase::handleStandardRequest(const SetupPacket& setup) {
    switch (setup.request) {
        case K_REQUEST_SET_ADDRESS:
            // The new address takes effect only after the status stage has been acknowledged.
            pending_address_ = static_cast<uint8_t>(setup.value & 0x7F);
            address_pending_ = true;
            sendControlStatus();
            break;

        case K_REQUEST_SET_CONFIGURATION:
            stats_.configured = (setup.value != 0);
            if (stats_.configured) {
                for (UsbFunctionBase* function : tables_.functions) {
                    function->onConfigured();
                }
            }
            sendControlStatus();
            break;

        case K_REQUEST_GET_CONFIGURATION:
            control_scratch_[0] = stats_.configured ? 1 : 0;
            sendControlData(std::span<const uint8_t>(control_scratch_, 1), setup.length);
            break;

        case K_REQUEST_GET_DESCRIPTOR:
            handleGetDescriptor(setup);
            break;

        case K_REQUEST_GET_STATUS:
            control_scratch_[0] = tables_.self_powered ? 1 : 0;
            control_scratch_[1] = 0;
            sendControlData(std::span<const uint8_t>(control_scratch_, 2), setup.length);
            break;

        case K_REQUEST_GET_INTERFACE:
            control_scratch_[0] = 0;  // only alternate setting 0 exists
            sendControlData(std::span<const uint8_t>(control_scratch_, 1), setup.length);
            break;

        case K_REQUEST_SET_INTERFACE:
        case K_REQUEST_CLEAR_FEATURE:
        case K_REQUEST_SET_FEATURE:
            sendControlStatus();  // accepted; no alternate settings or features are managed
            break;

        default:
            stallControl();
            break;
    }
}

void UsbDeviceBase::handleGetDescriptor(const SetupPacket& setup) {
    const uint8_t type  = static_cast<uint8_t>(setup.value >> 8);
    const uint8_t index = static_cast<uint8_t>(setup.value & 0xFF);

    switch (type) {
        case K_DESCRIPTOR_TYPE_DEVICE:
            sendControlData(tables_.device_descriptor, setup.length);
            break;

        case K_DESCRIPTOR_TYPE_CONFIGURATION:
            sendControlData(tables_.configuration_descriptor, setup.length);
            break;

        case K_DESCRIPTOR_TYPE_STRING: {
            const uint16_t length = buildStringDescriptor(index, control_scratch_);
            if (length == 0) {
                stallControl();
            } else {
                sendControlData(std::span<const uint8_t>(control_scratch_, length), setup.length);
            }
            break;
        }

        default:
            // Device qualifier and anything else: a full-speed-only device stalls these.
            stallControl();
            break;
    }
}

// Builds string descriptor `index` into `out` as UTF-16LE from the ASCII table. Returns 0 if no such string.
uint16_t UsbDeviceBase::buildStringDescriptor(uint8_t index, std::span<uint8_t> out) {
    if (index == 0) {
        out[0] = 4;
        out[1] = K_DESCRIPTOR_TYPE_STRING;
        out[2] = static_cast<uint8_t>(K_LANGUAGE_ID_ENGLISH_US & 0xFF);
        out[3] = static_cast<uint8_t>(K_LANGUAGE_ID_ENGLISH_US >> 8);
        return 4;
    }
    if (index >= tables_.strings.size() || tables_.strings[index] == nullptr) {
        return 0;
    }
    const char*  text      = tables_.strings[index];
    const size_t max_chars = (out.size() - 2) / 2;
    size_t       count     = 0;
    while (text[count] != '\0' && count < max_chars) {
        out[2 + count * 2]     = static_cast<uint8_t>(text[count]);
        out[2 + count * 2 + 1] = 0;
        count++;
    }
    out[0] = static_cast<uint8_t>(2 + count * 2);
    out[1] = K_DESCRIPTOR_TYPE_STRING;
    return out[0];
}

void UsbDeviceBase::handleClassRequest(const SetupPacket& setup) {
    UsbFunctionBase* function = findFunctionForInterface(static_cast<uint8_t>(setup.index & 0xFF));
    if (function == nullptr) {
        stallControl();
        return;
    }

    if (setup.request_type & K_REQUEST_DIRECTION_IN) {
        const int written = function->onControlIn(setup, control_scratch_);
        if (written < 0) {
            stallControl();
            return;
        }
        sendControlData(std::span<const uint8_t>(control_scratch_, static_cast<size_t>(written)), setup.length);
    } else if (setup.length == 0) {
        if (function->onControlOut(setup, {})) {
            sendControlStatus();
        } else {
            stallControl();
        }
    } else {
        // The host will send one data packet; deliver it on completion, then acknowledge.
        control_stage_ = ControlStage::data_out;
        armControlOut(setup.length > K_FULL_SPEED_MAX_PACKET_SIZE ? K_FULL_SPEED_MAX_PACKET_SIZE : setup.length);
    }
}

// Starts a device-to-host data stage. `data` must stay valid until the transfer completes (descriptors are
// constexpr tables; short replies are staged in control_scratch_).
void UsbDeviceBase::sendControlData(std::span<const uint8_t> data, uint16_t requested_length) {
    if (requested_length == 0) {
        // No data stage was requested: the transfer ends with a device-to-host zero-length status packet.
        sendControlStatus();
        return;
    }
    uint16_t length = static_cast<uint16_t>(data.size());
    if (length > requested_length) {
        length = requested_length;  // never send more than the host asked for
    }
    control_in_pointer_   = data.data();
    control_in_remaining_ = length;
    // If we send less than requested and the last packet is full-sized, a zero-length packet must follow so the
    // host knows the data stage is over.
    control_in_needs_zlp_ =
        (length < requested_length) && (length % K_FULL_SPEED_MAX_PACKET_SIZE == 0) && (length != 0);
    control_stage_ = ControlStage::data_in;
    pumpControlIn();
}

void UsbDeviceBase::pumpControlIn() {
    uint16_t chunk = control_in_remaining_;
    if (chunk > K_FULL_SPEED_MAX_PACKET_SIZE) {
        chunk = K_FULL_SPEED_MAX_PACKET_SIZE;
    }

    if (chunk == 0 && !control_in_needs_zlp_) {
        // Data stage done: the host acknowledges with a zero-length OUT packet.
        control_stage_ = ControlStage::status_out;
        armControlOut(0);
        return;
    }

    if (chunk > 0) {
        memcpy(usb_dpram->ep0_buf_a, control_in_pointer_, chunk);
    }
    control_in_pointer_ += chunk;
    control_in_remaining_ = static_cast<uint16_t>(control_in_remaining_ - chunk);
    if (chunk < K_FULL_SPEED_MAX_PACKET_SIZE) {
        control_in_needs_zlp_ = false;  // this (short or zero-length) packet terminates the stage
    }

    uint32_t value = chunk | USB_BUF_CTRL_FULL | USB_BUF_CTRL_AVAIL;
    if (control_in_data1_) {
        value |= USB_BUF_CTRL_DATA1_PID;
    }
    control_in_data1_ = !control_in_data1_;
    writeBufferControl(&usb_dpram->ep_buf_ctrl[0].in, value);
}

// Zero-length DATA1 IN packet: the status stage of a host-to-device or no-data control transfer.
void UsbDeviceBase::sendControlStatus() {
    control_stage_    = ControlStage::status_in;
    control_in_data1_ = true;
    writeBufferControl(&usb_dpram->ep_buf_ctrl[0].in, USB_BUF_CTRL_FULL | USB_BUF_CTRL_AVAIL | USB_BUF_CTRL_DATA1_PID);
}

void UsbDeviceBase::armControlOut(uint16_t length) {
    uint32_t value = length | USB_BUF_CTRL_AVAIL;
    if (control_out_data1_) {
        value |= USB_BUF_CTRL_DATA1_PID;
    }
    control_out_data1_ = !control_out_data1_;
    writeBufferControl(&usb_dpram->ep_buf_ctrl[0].out, value);
}

void UsbDeviceBase::stallControl() {
    stats_.control_stalls++;
    usb_dpram->ep_buf_ctrl[0].in  = USB_BUF_CTRL_STALL;
    usb_dpram->ep_buf_ctrl[0].out = USB_BUF_CTRL_STALL;
    USB_HW_SET->ep_stall_arm      = USB_EP_STALL_ARM_EP0_IN_BITS | USB_EP_STALL_ARM_EP0_OUT_BITS;
    control_stage_                = ControlStage::idle;
}

void UsbDeviceBase::onControlInComplete() {
    switch (control_stage_) {
        case ControlStage::data_in:
            pumpControlIn();
            break;
        case ControlStage::status_in:
            if (address_pending_) {
                usb_hw->dev_addr_ctrl = pending_address_;
                address_pending_      = false;
            }
            control_stage_ = ControlStage::idle;
            break;
        default:
            control_stage_ = ControlStage::idle;
            break;
    }
}

void UsbDeviceBase::onControlOutComplete(uint16_t length) {
    switch (control_stage_) {
        case ControlStage::status_out:
            control_stage_ = ControlStage::idle;
            break;

        case ControlStage::data_out: {
            UsbFunctionBase* function = findFunctionForInterface(static_cast<uint8_t>(current_setup_.index & 0xFF));
            const bool       handled =
                function != nullptr &&
                function->onControlOut(current_setup_, std::span<const uint8_t>(usb_dpram->ep0_buf_a, length));
            if (handled) {
                sendControlStatus();
            } else {
                stallControl();
            }
            break;
        }

        default:
            control_stage_ = ControlStage::idle;
            break;
    }
}

}  // namespace drivers::usb
