// CDC channel tests: the virtual serial ports seen through BufferedSerialCommunicationInterface.
#include <gtest/gtest.h>

#include <array>
#include <cstring>
#include <string>
#include <vector>

#include "drivers/interfaces/BufferedSerialCommunicationInterface.h"
#include "drivers/usb/UsbCdcChannel.h"
#include "drivers/usb/UsbDevice.h"
#include "fake_hardware.h"
#include "fake_host.h"

using namespace drivers::usb;

namespace {

constexpr DeviceConfig K_CONFIG{.vendor_id = 0x2E8A, .product_id = 0x000A, .tx_block_timeout_us = 5000};
using Device                  = UsbDevice<K_CONFIG, Cdc<256, 256>, Cdc<128, 128>>;

// Endpoint numbers the plan assigns (see descriptor_plan_test.cpp).
constexpr uint8_t K_PORT0_OUT = 1, K_PORT0_IN = 2, K_PORT1_OUT = 2, K_PORT1_IN = 4;
constexpr uint8_t K_PORT0_CONTROL_INTERFACE = 0, K_PORT1_CONTROL_INTERFACE = 2;

std::string          toString(const std::vector<uint8_t>& v) { return std::string(v.begin(), v.end()); }
std::vector<uint8_t> toBytes(const std::string& s) { return std::vector<uint8_t>(s.begin(), s.end()); }

class CdcChannel : public ::testing::Test {
protected:
    void SetUp() override {
        fake_hardware::reset();
        device = new Device({.manufacturer = "M", .product = "P", .function_names = {"Protocol", "Debug"}});
        device->init();
        host = new FakeHost(*device);
        host->enumerate();
        port0 = &device->channel<0>();
        port1 = &device->channel<1>();
    }
    void TearDown() override {
        delete host;
        delete device;
    }

    // Collect everything the device has queued on an IN endpoint, packet by packet.
    std::vector<uint8_t> drainIn(uint8_t endpoint, std::vector<int>* sizes = nullptr) {
        std::vector<uint8_t> all;
        for (int guard = 0; guard < 64 && FakeHost::inArmed(endpoint); guard++) {
            std::vector<uint8_t> packet = host->collectIn(endpoint);
            if (sizes) sizes->push_back(static_cast<int>(packet.size()));
            all.insert(all.end(), packet.begin(), packet.end());
        }
        return all;
    }

    Device*                                                    device = nullptr;
    FakeHost*                                                  host   = nullptr;
    drivers::interfaces::BufferedSerialCommunicationInterface* port0  = nullptr;
    drivers::interfaces::BufferedSerialCommunicationInterface* port1  = nullptr;
};

}  // namespace

/// ------------------------------ class requests ------------------------------

TEST_F(CdcChannel, lineCodingRoundTrips) {
    host->setLineCoding(K_PORT0_CONTROL_INTERFACE, 9600, 7);
    EXPECT_FALSE(host->controlIsStalled());
    EXPECT_EQ(device->channel<0>().getLineCoding().baud_rate, 9600u);
    EXPECT_EQ(device->channel<0>().getLineCoding().data_bits, 7);

    const std::vector<uint8_t> read = host->controlTransferIn(FakeHost::K_IN_CLASS_ITF, cdc::K_REQUEST_GET_LINE_CODING,
                                                              0, K_PORT0_CONTROL_INTERFACE, 7);
    ASSERT_EQ(read.size(), 7u);
    uint32_t baud = 0;
    memcpy(&baud, read.data(), 4);
    EXPECT_EQ(baud, 9600u);
    EXPECT_EQ(device->channel<1>().getLineCoding().baud_rate, 115200u) << "must not leak to the other port";
}

TEST_F(CdcChannel, controlLineStateIsRoutedToTheRightPort) {
    host->setControlLineState(K_PORT1_CONTROL_INTERFACE, true, false);
    EXPECT_TRUE(device->channel<1>().getDtr());
    EXPECT_FALSE(device->channel<1>().getRts());
    EXPECT_FALSE(device->channel<0>().getDtr());
}

TEST_F(CdcChannel, sendBreakIsAccepted) {
    host->controlTransferOut(FakeHost::K_OUT_CLASS_ITF, cdc::K_REQUEST_SEND_BREAK, 0xFFFF, K_PORT0_CONTROL_INTERFACE);
    EXPECT_FALSE(host->controlIsStalled());
}

/// ------------------------------ connection state and transmit policy ------------------------------

TEST_F(CdcChannel, notConnectedUntilHostAssertsDtr) {
    EXPECT_FALSE(device->channel<0>().isConnected());
    host->openPort(K_PORT0_CONTROL_INTERFACE);
    EXPECT_TRUE(device->channel<0>().isConnected());
    EXPECT_FALSE(device->channel<1>().isConnected());
}

TEST_F(CdcChannel, transmitWhileNobodyIsListeningDropsInsteadOfBlocking) {
    // No DTR: a UART would spin here forever once its ring filled. We must return immediately and count.
    for (int i = 0; i < 1000; i++) port0->transmitByte('x');
    EXPECT_EQ(device->getStats().tx_drops, 1000u);
    EXPECT_FALSE(FakeHost::inArmed(K_PORT0_IN)) << "nothing may be queued for a host that is not reading";
}

TEST_F(CdcChannel, suspendedBusCountsAsDisconnected) {
    host->openPort(K_PORT0_CONTROL_INTERFACE);
    ASSERT_TRUE(device->channel<0>().isConnected());
    host->suspend();
    EXPECT_FALSE(device->channel<0>().isConnected());
    port0->transmitByte('x');
    EXPECT_EQ(device->getStats().tx_drops, 1u);
}

/// ------------------------------ device -> host ------------------------------

TEST_F(CdcChannel, transmitBytesArriveAtTheHost) {
    host->openPort(K_PORT0_CONTROL_INTERFACE);
    std::string message = "hello host\r\n";
    port0->transmitBytes(std::span<uint8_t>(reinterpret_cast<uint8_t*>(message.data()), message.size()));
    EXPECT_TRUE(FakeHost::inArmed(K_PORT0_IN));
    EXPECT_EQ(toString(drainIn(K_PORT0_IN)), message);
    EXPECT_FALSE(FakeHost::inArmed(K_PORT0_IN)) << "must not re-arm with an empty ring";
    EXPECT_EQ(device->getStats().tx_drops, 0u);
}

TEST_F(CdcChannel, largeTransmitIsChunkedIntoFullPackets) {
    host->openPort(K_PORT0_CONTROL_INTERFACE);
    std::vector<uint8_t> payload(200);
    for (size_t i = 0; i < payload.size(); i++) payload[i] = static_cast<uint8_t>(i);
    port0->transmitBytes(payload);
    std::vector<int>           sizes;
    const std::vector<uint8_t> received = drainIn(K_PORT0_IN, &sizes);
    EXPECT_EQ(received, payload);
    EXPECT_EQ(sizes, (std::vector<int>{64, 64, 64, 8}));
    // Bulk endpoints start at DATA0 after SET_CONFIGURATION and toggle per packet.
    EXPECT_EQ(host->in_pids[K_PORT0_IN], (std::vector<bool>{false, true, false, true}));
}

TEST_F(CdcChannel, bulkOutPidTogglesPerPacketAndRestartsAtData0AfterReset) {
    EXPECT_FALSE(FakeHost::outExpectsData1(K_PORT0_OUT)) << "first OUT after configuration is DATA0";
    host->sendOut(K_PORT0_OUT, "a");
    EXPECT_TRUE(FakeHost::outExpectsData1(K_PORT0_OUT));
    host->sendOut(K_PORT0_OUT, "b");
    EXPECT_FALSE(FakeHost::outExpectsData1(K_PORT0_OUT));
    host->busReset();
    host->enumerate();
    EXPECT_FALSE(FakeHost::outExpectsData1(K_PORT0_OUT)) << "toggle must restart after a bus reset";
}

TEST_F(CdcChannel, transmitWaitsBoundedTimeWhenRingIsFullThenDrops) {
    host->openPort(K_PORT0_CONTROL_INTERFACE);
    // One full packet goes straight to the endpoint; the host never collects it. The ring (255 usable) then
    // absorbs 255 more bytes without loss.
    std::vector<uint8_t> first_packet(64, 'a');
    port0->transmitBytes(first_packet);
    ASSERT_TRUE(FakeHost::inArmed(K_PORT0_IN));
    for (int i = 0; i < 255; i++) port0->transmitByte('a');
    ASSERT_EQ(device->getStats().tx_drops, 0u) << "endpoint + ring must absorb 64 + 255 bytes";

    // The next byte cannot fit. With the fake clock advancing 1 us per read, the wait must give up after
    // tx_block_timeout_us and count a drop rather than hang.
    fake_hardware::setClockAutoAdvanceUs(1);
    port0->transmitByte('b');
    EXPECT_EQ(device->getStats().tx_drops, 1u);

    // Once the host starts reading again, transmit flows.
    EXPECT_EQ(host->collectIn(K_PORT0_IN).size(), 64u);
    port0->transmitByte('c');
    EXPECT_EQ(device->getStats().tx_drops, 1u);
    const std::vector<uint8_t> rest = drainIn(K_PORT0_IN);
    EXPECT_EQ(rest.size(), 255u + 1u);
    EXPECT_EQ(rest.back(), 'c');
}

TEST_F(CdcChannel, flushTxReturnsOnceHostHasCollectedEverything) {
    host->openPort(K_PORT0_CONTROL_INTERFACE);
    port0->transmitByte('z');
    // Host collects while flush is waiting: emulate by collecting first, then flushing (single-threaded test).
    host->collectIn(K_PORT0_IN);
    fake_hardware::setClockAutoAdvanceUs(1);
    device->channel<0>().flushTx();  // must return promptly: ring empty and endpoint idle
    EXPECT_FALSE(FakeHost::inArmed(K_PORT0_IN));
}

/// ------------------------------ host -> device ------------------------------

TEST_F(CdcChannel, receivedBytesAreReadableThroughTheInterface) {
    host->sendOut(K_PORT0_OUT, "ping");
    EXPECT_EQ(port0->getReceivedBytesAvailableAmount(), 4u);
    EXPECT_EQ(port0->readReceivedByte(), 'p');
    std::array<uint8_t, 8> buffer{};
    EXPECT_EQ(port0->readReceivedBytes(buffer), 3u);
    EXPECT_EQ(std::string(buffer.begin(), buffer.begin() + 3), "ing");
    EXPECT_EQ(port0->getReceivedBytesAvailableAmount(), 0u);
    EXPECT_TRUE(FakeHost::outArmed(K_PORT0_OUT)) << "must re-arm after delivering a packet";
}

TEST_F(CdcChannel, portsHaveIndependentReceiveBuffers) {
    host->sendOut(K_PORT0_OUT, "zero");
    host->sendOut(K_PORT1_OUT, "one");
    EXPECT_EQ(port0->getReceivedBytesAvailableAmount(), 4u);
    EXPECT_EQ(port1->getReceivedBytesAvailableAmount(), 3u);
    std::array<uint8_t, 8> b{};
    port1->readReceivedBytes(b);
    EXPECT_EQ(std::string(b.begin(), b.begin() + 3), "one");
}

TEST_F(CdcChannel, receiveIsFlowControlledNotLossy) {
    // Port 1 has a 128-byte ring (127 usable). Deliver full packets until the driver stops arming.
    std::vector<uint8_t> packet(64, 'r');
    int                  delivered = 0;
    while (FakeHost::outArmed(K_PORT1_OUT) && delivered < 10) {
        host->sendOut(K_PORT1_OUT, packet.data(), 64);
        delivered++;
    }
    EXPECT_EQ(delivered, 1) << "127 usable bytes: after one 64-byte packet there is no room for another";
    EXPECT_FALSE(FakeHost::outArmed(K_PORT1_OUT)) << "must NAK the host rather than accept data it cannot store";
    EXPECT_EQ(device->getStats().rx_overruns, 0u);
    EXPECT_EQ(port1->getReceivedBytesAvailableAmount(), 64u);

    // Reading makes room and re-arms.
    std::array<uint8_t, 64> sink{};
    port1->readReceivedBytes(sink);
    EXPECT_TRUE(FakeHost::outArmed(K_PORT1_OUT));
}

/// ------------------------------ lifecycle ------------------------------

TEST_F(CdcChannel, busResetForgetsBufferedDataAndConnectionState) {
    host->openPort(K_PORT0_CONTROL_INTERFACE);
    host->sendOut(K_PORT0_OUT, "stale");
    port0->transmitByte('q');
    host->busReset();
    EXPECT_FALSE(device->channel<0>().isConnected());
    EXPECT_EQ(port0->getReceivedBytesAvailableAmount(), 0u);
    host->enumerate();
    EXPECT_TRUE(FakeHost::outArmed(K_PORT0_OUT)) << "re-armed after re-enumeration";
    EXPECT_FALSE(FakeHost::inArmed(K_PORT0_IN)) << "old queued byte must not be sent to the new session";
}

TEST_F(CdcChannel, bootloaderTouchRebootsOnTwelveHundredBaudThenDtrDrop) {
    host->openPort(K_PORT0_CONTROL_INTERFACE, 115200);
    host->setControlLineState(K_PORT0_CONTROL_INTERFACE, false);  // DTR drop at 115200: nothing happens
    host->setLineCoding(K_PORT0_CONTROL_INTERFACE, 1200);
    host->setControlLineState(K_PORT0_CONTROL_INTERFACE, true);
    EXPECT_THROW(host->setControlLineState(K_PORT0_CONTROL_INTERFACE, false), fake_hardware::BootloaderRequested);
}

TEST(CdcChannelBootloaderTouchDisabled, twelveHundredBaudIsIgnoredWhenDisabled) {
    constexpr DeviceConfig K_NO_TOUCH{.vendor_id = 1, .product_id = 1, .bootloader_touch_enabled = false};
    fake_hardware::reset();
    UsbDevice<K_NO_TOUCH, Cdc<128, 128>> device({.manufacturer = "M", .product = "P", .function_names = {"A"}});
    device.init();
    FakeHost host(device);
    host.enumerate();
    host.openPort(0, 1200);
    EXPECT_NO_THROW(host.setControlLineState(0, false));
}
