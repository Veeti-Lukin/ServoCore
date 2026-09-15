// Enumeration tests: the device driven by a simulated host through the full control-transfer sequence.
#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "drivers/usb/UsbCdcChannel.h"
#include "drivers/usb/UsbDevice.h"
#include "fake_hardware.h"
#include "fake_host.h"
#include "hardware/gpio.h"
#include "hardware/structs/usb.h"
#include "hardware/structs/usb_dpram.h"

using namespace drivers::usb;

namespace {

constexpr DeviceConfig K_CONFIG{.vendor_id = 0x2E8A, .product_id = 0x000A};
// Exactly 31 characters: its string descriptor is 2 + 62 = 64 bytes, one full packet, which forces a ZLP.
constexpr const char* K_LONG_PRODUCT = "0123456789012345678901234567890";

using Device                         = UsbDevice<K_CONFIG, Cdc<256, 256>, Cdc<128, 128>>;

std::string utf16leToAscii(const std::vector<uint8_t>& descriptor) {
    std::string out;
    for (size_t i = 2; i + 1 < descriptor.size(); i += 2) out += static_cast<char>(descriptor[i]);
    return out;
}

class Enumeration : public ::testing::Test {
protected:
    void SetUp() override {
        fake_hardware::reset();
        device = new Device(
            {.manufacturer = "Veeti Lukin", .product = K_LONG_PRODUCT, .function_names = {"Protocol", "Debug"}});
        device->init();
        host = new FakeHost(*device);
    }
    void TearDown() override {
        delete host;
        delete device;
    }
    Device*   device = nullptr;
    FakeHost* host   = nullptr;
};

}  // namespace

TEST_F(Enumeration, initBringsUpControllerAndConnectsWithoutVbusSense) {
    EXPECT_TRUE(usb_hw->main_ctrl & USB_MAIN_CTRL_CONTROLLER_EN_BITS);
    EXPECT_TRUE(usb_hw->sie_ctrl & USB_SIE_CTRL_PULLUP_EN_BITS) << "no VBUS pin: must connect unconditionally";
    EXPECT_TRUE(usb_hw->inte & USB_INTS_SETUP_REQ_BITS);
    EXPECT_TRUE(usb_hw->inte & USB_INTS_DEV_SUSPEND_BITS);
    EXPECT_TRUE(device->isVbusPresent());
    // All six endpoints programmed with distinct buffers.
    for (const EndpointPlan& ep : Device::Plan::K_ENDPOINTS) {
        const uint32_t control =
            ep.isIn() ? usb_dpram->ep_ctrl[ep.number() - 1].in : usb_dpram->ep_ctrl[ep.number() - 1].out;
        EXPECT_TRUE(control & EP_CTRL_ENABLE_BITS) << "endpoint " << int(ep.address);
        EXPECT_EQ(control & 0xFFFF, ep.dpram_offset);
        EXPECT_EQ((control >> EP_CTRL_BUFFER_TYPE_LSB) & 0x3, ep.transfer_type);
    }
}

TEST_F(Enumeration, deviceDescriptorIsServedFromThePlan) {
    const std::vector<uint8_t> d = host->getDescriptor(K_DESCRIPTOR_TYPE_DEVICE, 0, 64);
    ASSERT_EQ(d.size(), 18u);
    EXPECT_EQ(std::vector<uint8_t>(Device::Plan::K_DEVICE_DESCRIPTOR.begin(), Device::Plan::K_DEVICE_DESCRIPTOR.end()),
              d);
}

TEST_F(Enumeration, setAddressTakesEffectOnlyAfterStatusStage) {
    host->sendSetup(FakeHost::K_OUT_DEVICE, K_REQUEST_SET_ADDRESS, 7, 0, 0);
    EXPECT_EQ(usb_hw->dev_addr_ctrl, 0u) << "address applied before the status stage";
    bool data1 = false;
    host->collectControlIn(&data1);
    EXPECT_TRUE(data1) << "status stage must be DATA1";
    EXPECT_EQ(usb_hw->dev_addr_ctrl, 7u);
}

TEST_F(Enumeration, configurationDescriptorIsSentInFullOverSeveralPackets) {
    std::vector<int>           sizes;
    const std::vector<uint8_t> d = host->getDescriptor(K_DESCRIPTOR_TYPE_CONFIGURATION, 0, 255, 0, &sizes);
    ASSERT_EQ(d.size(), Device::Plan::K_CONFIGURATION_DESCRIPTOR.size());
    EXPECT_EQ(std::vector<uint8_t>(Device::Plan::K_CONFIGURATION_DESCRIPTOR.begin(),
                                   Device::Plan::K_CONFIGURATION_DESCRIPTOR.end()),
              d);
    EXPECT_EQ(sizes, (std::vector<int>{64, 64, 13}));
    // Data stage starts with DATA1 and toggles; a host drops packets with the wrong PID as retransmissions.
    EXPECT_EQ(host->in_pids[0], (std::vector<bool>{true, false, true}));
}

TEST_F(Enumeration, controlOutDataStageExpectsData1) {
    host->enumerate();
    host->sendSetup(FakeHost::K_OUT_CLASS_ITF, cdc::K_REQUEST_SET_LINE_CODING, 0, 0, 7);
    ASSERT_TRUE(host->controlOutArmed());
    EXPECT_TRUE(FakeHost::outExpectsData1(0)) << "first packet after SETUP is always DATA1";
    // Status stage after an IN data stage is a DATA1 OUT as well.
    host->sendSetup(FakeHost::K_IN_DEVICE, K_REQUEST_GET_DESCRIPTOR, K_DESCRIPTOR_TYPE_DEVICE << 8, 0, 18);
    host->collectControlIn();
    ASSERT_TRUE(host->controlOutArmed());
    EXPECT_TRUE(FakeHost::outExpectsData1(0));
}

TEST_F(Enumeration, hostAskingForOnlyNineBytesGetsExactlyNine) {
    std::vector<int>           sizes;
    const std::vector<uint8_t> d = host->getDescriptor(K_DESCRIPTOR_TYPE_CONFIGURATION, 0, 9, 0, &sizes);
    EXPECT_EQ(d.size(), 9u);
    EXPECT_EQ(sizes, (std::vector<int>{9}));
}

TEST_F(Enumeration, fullPacketShortTransferIsTerminatedWithZeroLengthPacket) {
    std::vector<int>           sizes;
    const std::vector<uint8_t> d = host->getDescriptor(K_DESCRIPTOR_TYPE_STRING, Device::Plan::K_STRING_INDEX_PRODUCT,
                                                       255, K_LANGUAGE_ID_ENGLISH_US, &sizes);
    EXPECT_EQ(d.size(), 64u);
    EXPECT_EQ(sizes, (std::vector<int>{64, 0})) << "64 bytes of a 255-byte request needs a terminating ZLP";
    EXPECT_EQ(utf16leToAscii(d), K_LONG_PRODUCT);
}

TEST_F(Enumeration, zeroLengthRequestEndsWithDeviceToHostStatus) {
    std::vector<int> sizes;
    host->getDescriptor(K_DESCRIPTOR_TYPE_DEVICE, 0, 0, 0, &sizes);
    EXPECT_EQ(sizes, (std::vector<int>{0})) << "wLength 0: no data stage, status is an IN ZLP";
}

TEST_F(Enumeration, stringDescriptorsIncludeLanguageNamesAndUniqueIdSerial) {
    const std::vector<uint8_t> lang = host->getDescriptor(K_DESCRIPTOR_TYPE_STRING, 0, 255);
    EXPECT_EQ(lang, (std::vector<uint8_t>{0x04, 0x03, 0x09, 0x04}));

    EXPECT_EQ(utf16leToAscii(host->getDescriptor(K_DESCRIPTOR_TYPE_STRING, 1, 255, 0x0409)), "Veeti Lukin");
    EXPECT_EQ(utf16leToAscii(host->getDescriptor(K_DESCRIPTOR_TYPE_STRING, 3, 255, 0x0409)), "E66138528B123456")
        << "serial must come from the chip unique ID when none is supplied";
    EXPECT_EQ(utf16leToAscii(host->getDescriptor(K_DESCRIPTOR_TYPE_STRING, 4, 255, 0x0409)), "Protocol");
    EXPECT_EQ(utf16leToAscii(host->getDescriptor(K_DESCRIPTOR_TYPE_STRING, 5, 255, 0x0409)), "Debug");

    host->getDescriptor(K_DESCRIPTOR_TYPE_STRING, 6, 255, 0x0409);
    EXPECT_TRUE(host->controlIsStalled()) << "unknown string index must stall";
}

TEST_F(Enumeration, setConfigurationConfiguresDeviceAndArmsBothPortsForReception) {
    host->enumerate();
    EXPECT_TRUE(device->isConfigured());
    EXPECT_TRUE(FakeHost::outArmed(1)) << "port 0 data OUT not armed";
    EXPECT_TRUE(FakeHost::outArmed(2)) << "port 1 data OUT not armed";
    EXPECT_FALSE(FakeHost::inArmed(2)) << "nothing to send yet";
}

TEST_F(Enumeration, unsupportedRequestsStall) {
    host->sendSetup(FakeHost::K_IN_DEVICE, 0x7F, 0, 0, 8);
    EXPECT_TRUE(host->controlIsStalled());
    EXPECT_NE(usb_hw->ep_stall_arm, 0u);
    EXPECT_EQ(device->getStats().control_stalls, 1u);

    host->getDescriptor(K_DESCRIPTOR_TYPE_DEVICE_QUALIFIER, 0, 10);
    EXPECT_TRUE(host->controlIsStalled()) << "full-speed-only device stalls the device qualifier";

    // Class request for an interface nobody owns.
    host->sendSetup(FakeHost::K_IN_CLASS_ITF, cdc::K_REQUEST_GET_LINE_CODING, 0, 9, 7);
    EXPECT_TRUE(host->controlIsStalled());
}

TEST_F(Enumeration, busResetClearsAddressAndConfiguration) {
    host->enumerate();
    ASSERT_TRUE(device->isConfigured());
    host->busReset();
    EXPECT_EQ(usb_hw->dev_addr_ctrl, 0u);
    EXPECT_FALSE(device->isConfigured());
    EXPECT_EQ(device->getStats().bus_resets, 2u);  // one inside enumerate(), one here
    // The device must enumerate again cleanly afterwards.
    host->enumerate();
    EXPECT_TRUE(device->isConfigured());
}

TEST_F(Enumeration, suspendAndResumeAreTracked) {
    host->enumerate();
    host->suspend();
    EXPECT_TRUE(device->isSuspended());
    EXPECT_TRUE(device->isConfigured()) << "suspend does not unconfigure";
    host->resume();
    EXPECT_FALSE(device->isSuspended());
    host->suspend();
    host->getDescriptor(K_DESCRIPTOR_TYPE_DEVICE, 0, 18);
    EXPECT_FALSE(device->isSuspended()) << "any SETUP traffic means the host is awake";
}

TEST(EnumerationWithVbusSense, pinSelectsTheDetectionMode) {
    fake_hardware::reset();
    Device none({.manufacturer = "M", .product = "P", .function_names = {"A", "B"}}, -1);
    EXPECT_EQ(none.getVbusSenseMode(), UsbDeviceBase::VbusSense::none);
    // Every third GPIO from 1 can be muxed to the controller's VBUS_DETECT input.
    for (int pin : {1, 4, 7, 22, 25, 28}) {
        Device d({.manufacturer = "M", .product = "P", .function_names = {"A", "B"}}, pin);
        EXPECT_EQ(d.getVbusSenseMode(), UsbDeviceBase::VbusSense::controller_pin) << "GPIO " << pin;
    }
    for (int pin : {0, 2, 24, 26}) {
        Device d({.manufacturer = "M", .product = "P", .function_names = {"A", "B"}}, pin);
        EXPECT_EQ(d.getVbusSenseMode(), UsbDeviceBase::VbusSense::polled_gpio) << "GPIO " << pin;
    }
}

// GPIO 22 (= 1 mod 3): the controller reads VBUS itself and applies the pull-up in hardware.
TEST(EnumerationWithVbusSense, controllerPinModeLeavesGatingToTheHardware) {
    fake_hardware::reset();
    constexpr int K_VBUS_PIN = 22;
    Device        device({.manufacturer = "M", .product = "P", .function_names = {"A", "B"}}, K_VBUS_PIN);

    usb_hw->sie_status = 0;  // no VBUS at the controller's input
    device.init();
    EXPECT_EQ(fake_hardware::gpioFunction(K_VBUS_PIN), GPIO_FUNC_USB) << "pin must be muxed to the controller";
    EXPECT_EQ(usb_hw->pwr & USB_USB_PWR_VBUS_DETECT_OVERRIDE_EN_BITS, 0u) << "override would defeat the pin";
    EXPECT_TRUE(usb_hw->sie_ctrl & USB_SIE_CTRL_PULLUP_EN_BITS)
        << "pull-up is requested up front; the controller only applies it while VBUS_DETECTED";
    EXPECT_FALSE(usb_hw->inte & USB_INTS_VBUS_DETECT_BITS)
        << "VBUS_DETECT is a level interrupt on a read-only status bit and would storm";
    EXPECT_FALSE(device.isVbusPresent());

    FakeHost host(device);
    usb_hw->sie_status |= USB_SIE_STATUS_VBUS_DETECTED_BITS;  // cable plugged in
    device.run();
    EXPECT_TRUE(device.isVbusPresent());
    host.enumerate();
    host.openPort(0);
    EXPECT_TRUE(device.channel<0>().isConnected());

    usb_hw->sie_status &= ~USB_SIE_STATUS_VBUS_DETECTED_BITS;  // cable pulled
    device.run();
    EXPECT_FALSE(device.isVbusPresent());
    EXPECT_FALSE(device.isConfigured());
    EXPECT_FALSE(device.channel<0>().isConnected());
    EXPECT_EQ(usb_hw->dev_addr_ctrl, 0u);
    EXPECT_TRUE(usb_hw->sie_ctrl & USB_SIE_CTRL_PULLUP_EN_BITS) << "software must not touch the pull-up in this mode";

    usb_hw->sie_status |= USB_SIE_STATUS_VBUS_DETECTED_BITS;  // plugged in again
    device.run();
    host.enumerate();
    EXPECT_TRUE(device.isConfigured());
}

// GPIO 24 (the original Pico's VBUS sense pin, = 0 mod 3): read by software, pull-up gated from run().
TEST(EnumerationWithVbusSense, polledGpioModeGatesPullupFromRun) {
    fake_hardware::reset();
    constexpr int K_VBUS_PIN = 24;
    Device        device({.manufacturer = "M", .product = "P", .function_names = {"A", "B"}}, K_VBUS_PIN);

    fake_hardware::setGpioInput(K_VBUS_PIN, false);
    device.init();
    EXPECT_EQ(device.getVbusSenseMode(), UsbDeviceBase::VbusSense::polled_gpio);
    EXPECT_TRUE(usb_hw->pwr & USB_USB_PWR_VBUS_DETECT_OVERRIDE_EN_BITS);
    EXPECT_TRUE(fake_hardware::gpioIsInput(K_VBUS_PIN));
    EXPECT_FALSE(usb_hw->sie_ctrl & USB_SIE_CTRL_PULLUP_EN_BITS) << "must not present the pull-up without VBUS";
    EXPECT_FALSE(device.isVbusPresent());

    FakeHost host(device);
    fake_hardware::setGpioInput(K_VBUS_PIN, true);
    device.run();
    EXPECT_TRUE(usb_hw->sie_ctrl & USB_SIE_CTRL_PULLUP_EN_BITS);
    EXPECT_TRUE(device.isVbusPresent());
    host.enumerate();
    EXPECT_TRUE(device.isConfigured());

    // Cable pulled: pull-up released, configuration dropped, ports told.
    fake_hardware::setGpioInput(K_VBUS_PIN, false);
    device.run();
    EXPECT_FALSE(usb_hw->sie_ctrl & USB_SIE_CTRL_PULLUP_EN_BITS);
    EXPECT_FALSE(device.isConfigured());
    EXPECT_FALSE(device.channel<0>().isConnected());
}
