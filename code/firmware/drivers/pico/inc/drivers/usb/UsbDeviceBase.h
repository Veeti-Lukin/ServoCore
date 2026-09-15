#ifndef USBDEVICEBASE_H
#define USBDEVICEBASE_H

#include <hardware/irq.h>

#include <cstddef>
#include <cstdint>
#include <span>

#include "drivers/usb/usb_descriptor_plan.h"
#include "drivers/usb/usb_protocol.h"

namespace drivers::usb {

class UsbDeviceBase;

/** @brief Runtime state of one non-control endpoint. One per entry of the compile-time endpoint table. */
struct EndpointState {
    const EndpointPlan* plan                 = nullptr;
    volatile uint32_t*  buffer_control       = nullptr;  // this endpoint's buffer control register in DPRAM
    volatile uint32_t*  endpoint_control     = nullptr;  // this endpoint's control register in DPRAM
    uint8_t*            buffer               = nullptr;  // this endpoint's packet buffer in DPRAM
    bool                next_packet_is_data1 = false;
    /// IN: a packet has been handed to the controller and not yet collected. OUT: armed to receive.
    bool busy                                = false;
};

/**
 * @brief What the device core needs from every function (CDC channel, ...) at runtime.
 *
 * The compile-time side of a function lives in its trait type (see usb_descriptor_plan.h); this is the runtime
 * side the core dispatches control requests and endpoint completions to.
 */
class UsbFunctionBase {
public:
    virtual ~UsbFunctionBase()                                                         = default;

    /** @brief Whether the given interface number belongs to this function. Used to route class requests. */
    [[nodiscard]] virtual bool ownsInterface(uint8_t interface_number) const           = 0;

    /**
     * @brief Class-specific control request with a device-to-host data stage.
     * @param setup    The setup packet.
     * @param response Buffer to write the response into (one control packet).
     * @return Number of bytes written, or -1 if the request is not supported (the device will STALL).
     */
    virtual int onControlIn(const SetupPacket& setup, std::span<uint8_t> response)     = 0;

    /**
     * @brief Class-specific control request with a host-to-device data stage, or no data stage.
     * @param setup The setup packet.
     * @param data  The data stage payload; empty when the request carries none.
     * @return true if handled, false if not supported (the device will STALL).
     */
    virtual bool onControlOut(const SetupPacket& setup, std::span<const uint8_t> data) = 0;

    /** @brief The host selected the configuration; endpoints are live. */
    virtual void onConfigured()                                                        = 0;

    /** @brief Bus reset, VBUS lost or host detached: forget all transfer state. */
    virtual void onDisconnected()                                                      = 0;

    /**
     * @brief A transfer on one of this function's endpoints finished. Called from the interrupt handler.
     * @param endpoint The endpoint.
     * @param length   Bytes transferred.
     */
    virtual void onEndpointComplete(EndpointState& endpoint, uint16_t length)          = 0;
};

/** @brief Counters and flags, meant to be exposed as signal parameters. */
struct UsbStats {
    uint32_t bus_resets     = 0;
    uint32_t setup_packets  = 0;
    uint32_t control_stalls = 0;
    uint32_t rx_overruns    = 0;  // received bytes that did not fit a channel's RX ring
    uint32_t tx_drops       = 0;  // bytes dropped by a channel because the host was not reading
    bool     vbus_present   = false;
    bool     configured     = false;
    bool     suspended      = false;
};

/**
 * @brief Hardware- and protocol-level USB device core for the RP2040 / RP2350 controller.
 *
 * Owns the controller registers and DPRAM, runs the endpoint-0 control state machine, serves descriptors and
 * dispatches everything else to the registered functions. It is deliberately not a template: the templated
 * UsbDevice facade computes the descriptor plan at compile time and hands the resulting tables in.
 *
 * Interrupt model: the application installs handleInterrupt() on K_NVIC_INTERRUPT_NUMBER, the same way the UART
 * drivers are wired in interrupt_service_routines.cpp. Anything touched from both the interrupt and the main
 * loop is protected with InterruptGuard.
 */
class UsbDeviceBase {
public:
    static constexpr unsigned int K_NVIC_INTERRUPT_NUMBER = USBCTRL_IRQ;

    /** @brief Tables produced by the compile-time plan and storage owned by the facade. */
    struct Tables {
        std::span<const EndpointPlan> endpoint_plans;
        std::span<EndpointState>      endpoint_states;
        std::span<UsbFunctionBase*>   functions;
        std::span<const uint8_t>      device_descriptor;
        std::span<const uint8_t>      configuration_descriptor;
        /// String table indexed by descriptor index. Slot 0 is unused; slot 3 (serial) may be nullptr to request
        /// the chip's unique ID.
        std::span<const char*> strings;
        bool                   self_powered             = true;
        bool                   bootloader_touch_enabled = true;
        uint32_t               tx_block_timeout_us      = 5000;
    };

    /**
     * @brief How the device learns whether a host's VBUS is present. Chosen from the pin number in the constructor.
     *
     * A self-powered device must only present its D+ pull-up while VBUS is present (USB 2.0 §7.1.5), otherwise
     * it back-feeds an unpowered host and never notices a pulled cable.
     */
    enum class VbusSense : uint8_t {
        /// No VBUS sense on the board: the controller is told VBUS is always present and the pull-up is always on.
        none,
        /// The pin is one the controller can read directly (GPIO 1, 4, 7, ... 28: number = 1 mod 3). The pin is
        /// muxed to the controller's VBUS_DETECT input and the controller applies the pull-up only while VBUS is
        /// present, without any software involved. run() only mirrors the status bit into state and stats.
        controller_pin,
        /// Any other GPIO: read from run(), which enables and disables the pull-up accordingly.
        polled_gpio,
    };

    /**
     * @param vbus_detect_pin GPIO wired to a VBUS divider, or -1 if the board has no VBUS sense. See VbusSense
     *                        for how the pin number selects the detection mode.
     */
    explicit UsbDeviceBase(int vbus_detect_pin);

    /** @brief Reset and configure the controller, program endpoints, and connect (when VBUS is present). */
    void init();
    /** @brief Disconnect from the bus and hold the controller in reset. */
    void deInit();

    /** @brief Main-loop housekeeping. Polls the VBUS pin when one is configured. Cheap; call every iteration. */
    void run();

    /** @brief Handle a USB controller interrupt. Install on K_NVIC_INTERRUPT_NUMBER. */
    void handleInterrupt();

    [[nodiscard]] VbusSense       getVbusSenseMode() const { return vbus_sense_; }
    [[nodiscard]] bool            isConfigured() const { return stats_.configured; }
    [[nodiscard]] bool            isSuspended() const { return stats_.suspended; }
    [[nodiscard]] bool            isVbusPresent() const { return stats_.vbus_present; }
    [[nodiscard]] const UsbStats& getStats() const { return stats_; }
    /** @brief Mutable access so the counters can be bound to (read-only) signal parameters. */
    [[nodiscard]] UsbStats& getStats() { return stats_; }

    /// ------------------------------ Services for functions ------------------------------

    /** @brief Masks the USB interrupt for its lifetime. */
    class InterruptGuard {
    public:
        InterruptGuard();
        ~InterruptGuard();
        InterruptGuard(const InterruptGuard&)            = delete;
        InterruptGuard& operator=(const InterruptGuard&) = delete;

    private:
        bool was_enabled_;
    };

    /** @brief Look up the runtime state of a function's endpoint. */
    EndpointState& getEndpoint(uint8_t function_index, uint8_t index_in_function);

    /** @brief Make an OUT endpoint ready to receive one packet. */
    void armOut(EndpointState& endpoint);
    /** @brief Hand one packet (at most the endpoint's max packet size) to the controller to send. */
    void startIn(EndpointState& endpoint, std::span<const uint8_t> data);

    [[nodiscard]] bool     isBootloaderTouchEnabled() const { return tables_.bootloader_touch_enabled; }
    [[nodiscard]] uint32_t getTxBlockTimeoutUs() const { return tables_.tx_block_timeout_us; }

    /** @brief Reboot into the ROM USB bootloader. Does not return. */
    [[noreturn]] void rebootToBootloader();

    void countTxDrop() { stats_.tx_drops++; }
    void countRxOverrun() { stats_.rx_overruns++; }

protected:
    /** @brief Hand over the descriptor tables and state storage. Called by the facade once its members exist. */
    void attachTables(const Tables& tables);

private:
    /// Stage of the current endpoint-0 control transfer.
    enum class ControlStage : uint8_t {
        idle,
        data_in,     // sending packets of a device-to-host data stage
        status_out,  // waiting for the host's zero-length OUT that ends an IN transfer
        data_out,    // waiting for the host's data packet of a host-to-device data stage
        status_in,   // sending the zero-length IN that ends an OUT or no-data transfer
    };

    // Hardware.
    static VbusSense vbusSenseModeForPin(int pin);
    void             resetController();
    void             programEndpoints();
    void             configureVbusSense();
    void             onVbusChanged(bool present);
    void             connect();
    void             disconnect();
    void             writeBufferControl(volatile uint32_t* buffer_control, uint32_t value);

    // Endpoint 0.
    void     handleSetupPacket();
    void     handleStandardRequest(const SetupPacket& setup);
    void     handleGetDescriptor(const SetupPacket& setup);
    void     handleClassRequest(const SetupPacket& setup);
    void     sendControlData(std::span<const uint8_t> data, uint16_t requested_length);
    void     pumpControlIn();
    void     sendControlStatus();
    void     armControlOut(uint16_t length);
    void     stallControl();
    void     onControlInComplete();
    void     onControlOutComplete(uint16_t length);
    uint16_t buildStringDescriptor(uint8_t index, std::span<uint8_t> out);

    // Bus events.
    void             onBusReset();
    void             resetTransferState();
    UsbFunctionBase* findFunctionForInterface(uint8_t interface_number);

    Tables    tables_;
    int       vbus_detect_pin_;
    VbusSense vbus_sense_;
    UsbStats  stats_;
    bool      pullup_enabled_     = false;

    // Endpoint-0 control transfer state.
    ControlStage   control_stage_ = ControlStage::idle;
    SetupPacket    current_setup_{};
    const uint8_t* control_in_pointer_                                 = nullptr;
    uint16_t       control_in_remaining_                               = 0;
    bool           control_in_needs_zlp_                               = false;
    bool           control_in_data1_                                   = true;
    bool           control_out_data1_                                  = true;
    bool           address_pending_                                    = false;
    uint8_t        pending_address_                                    = 0;
    uint8_t        control_scratch_[K_FULL_SPEED_MAX_PACKET_SIZE]      = {};

    // Serial number derived from the chip's unique ID, used when no serial string is supplied.
    static constexpr size_t K_UNIQUE_SERIAL_LENGTH                     = 16;
    char                    unique_serial_[K_UNIQUE_SERIAL_LENGTH + 1] = {};
};

}  // namespace drivers::usb

#endif  // USBDEVICEBASE_H
