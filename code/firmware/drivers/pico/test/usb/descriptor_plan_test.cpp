// Tests for the compile-time descriptor plan. Most assertions here are static_asserts: if the plan is wrong the
// test binary does not compile. The runtime tests pin the produced bytes as golden vectors so that any change to
// the descriptor layout is a deliberate one.
#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <vector>

#include "drivers/usb/UsbCdcChannel.h"
#include "drivers/usb/usb_descriptor_plan.h"

using namespace drivers::usb;

namespace {

constexpr DeviceConfig K_CONFIG{.vendor_id = 0x1234, .product_id = 0x5678, .device_version_bcd = 0x0102};

using OnePort  = DevicePlan<K_CONFIG, Cdc<128, 128>>;
using TwoPorts = DevicePlan<K_CONFIG, Cdc<256, 256>, Cdc<128, 128>>;

/// ------------------------------ compile-time structure ------------------------------

static_assert(OnePort::K_NUM_INTERFACES == 2);
static_assert(OnePort::K_NUM_ENDPOINTS == 3);
static_assert(OnePort::K_NUM_STRINGS == 5);  // lang, manufacturer, product, serial, port name
static_assert(OnePort::K_CONFIGURATION_DESCRIPTOR.size() == 9 + 66);
static_assert(OnePort::K_DEVICE_DESCRIPTOR.size() == 18);

static_assert(TwoPorts::K_NUM_INTERFACES == 4);
static_assert(TwoPorts::K_NUM_ENDPOINTS == 6);
static_assert(TwoPorts::K_NUM_IN_ENDPOINTS == 4);
static_assert(TwoPorts::K_NUM_OUT_ENDPOINTS == 2);
static_assert(TwoPorts::K_CONFIGURATION_DESCRIPTOR.size() == 9 + 2 * 66);

// Endpoint numbering: IN and OUT counters are independent and both start at 1.
static_assert(TwoPorts::K_ENDPOINTS[0].address == 0x81);  // port 0 notification
static_assert(TwoPorts::K_ENDPOINTS[1].address == 0x01);  // port 0 data OUT
static_assert(TwoPorts::K_ENDPOINTS[2].address == 0x82);  // port 0 data IN
static_assert(TwoPorts::K_ENDPOINTS[3].address == 0x83);  // port 1 notification
static_assert(TwoPorts::K_ENDPOINTS[4].address == 0x02);  // port 1 data OUT
static_assert(TwoPorts::K_ENDPOINTS[5].address == 0x84);  // port 1 data IN

// DPRAM buffers are consecutive 64-byte slots after the controller's own area.
static_assert(TwoPorts::K_ENDPOINTS[0].dpram_offset == 0x180);
static_assert(TwoPorts::K_ENDPOINTS[5].dpram_offset == 0x180 + 5 * 64);

// Interface and string assignment.
static_assert(TwoPorts::K_LAYOUTS[0].first_interface == 0);
static_assert(TwoPorts::K_LAYOUTS[1].first_interface == 2);
static_assert(TwoPorts::K_LAYOUTS[0].first_string_index == 4);
static_assert(TwoPorts::K_LAYOUTS[1].first_string_index == 5);
static_assert(TwoPorts::K_LAYOUTS[1].endpoint_addresses[UsbCdcChannelBase::K_ENDPOINT_DATA_IN] == 0x84);

// Device descriptor content.
static_assert(TwoPorts::K_DEVICE_DESCRIPTOR[8] == 0x34 && TwoPorts::K_DEVICE_DESCRIPTOR[9] == 0x12);    // idVendor
static_assert(TwoPorts::K_DEVICE_DESCRIPTOR[10] == 0x78 && TwoPorts::K_DEVICE_DESCRIPTOR[11] == 0x56);  // idProduct
static_assert(TwoPorts::K_DEVICE_DESCRIPTOR[12] == 0x02 && TwoPorts::K_DEVICE_DESCRIPTOR[13] == 0x01);  // bcdDevice
static_assert(TwoPorts::K_DEVICE_DESCRIPTOR[4] == 0xEF && TwoPorts::K_DEVICE_DESCRIPTOR[5] == 0x02 &&
              TwoPorts::K_DEVICE_DESCRIPTOR[6] == 0x01);  // IAD composite

/// ------------------------------ helpers ------------------------------

// Walk a configuration descriptor and check that its sub-descriptors tile it exactly.
template <size_t N>
bool descriptorChainTiles(const std::array<uint8_t, N>& d, int* count = nullptr) {
    size_t offset = 0;
    int    n      = 0;
    while (offset < d.size()) {
        if (d[offset] == 0) return false;
        offset += d[offset];
        n++;
    }
    if (count) *count = n;
    return offset == d.size();
}

template <size_t N>
std::vector<uint8_t> endpointAddresses(const std::array<uint8_t, N>& d) {
    std::vector<uint8_t> out;
    for (size_t offset = 0; offset + 2 < d.size(); offset += d[offset]) {
        if (d[offset + 1] == K_DESCRIPTOR_TYPE_ENDPOINT) out.push_back(d[offset + 2]);
    }
    return out;
}

}  // namespace

TEST(DescriptorPlan, configurationDescriptorChainTilesExactly) {
    int count = 0;
    EXPECT_TRUE(descriptorChainTiles(OnePort::K_CONFIGURATION_DESCRIPTOR, &count));
    EXPECT_EQ(count, 11);  // config + IAD + 2 interfaces + 4 functional + 3 endpoints
    EXPECT_TRUE(descriptorChainTiles(TwoPorts::K_CONFIGURATION_DESCRIPTOR, &count));
    EXPECT_EQ(count, 21);
}

TEST(DescriptorPlan, endpointAddressesInDescriptorMatchPlanAndAreUnique) {
    const std::vector<uint8_t> addresses = endpointAddresses(TwoPorts::K_CONFIGURATION_DESCRIPTOR);
    ASSERT_EQ(addresses.size(), TwoPorts::K_NUM_ENDPOINTS);
    for (size_t i = 0; i < addresses.size(); i++) {
        EXPECT_EQ(addresses[i], TwoPorts::K_ENDPOINTS[i].address) << "endpoint " << i;
        for (size_t j = i + 1; j < addresses.size(); j++) {
            EXPECT_NE(addresses[i], addresses[j]) << "duplicate endpoint address";
        }
    }
}

TEST(DescriptorPlan, interfaceNumbersInDescriptorAreConsecutive) {
    const auto& d        = TwoPorts::K_CONFIGURATION_DESCRIPTOR;
    uint8_t     expected = 0;
    for (size_t offset = 0; offset + 2 < d.size(); offset += d[offset]) {
        if (d[offset + 1] == K_DESCRIPTOR_TYPE_INTERFACE) {
            EXPECT_EQ(d[offset + 2], expected++);
        }
    }
    EXPECT_EQ(expected, TwoPorts::K_NUM_INTERFACES);
}

TEST(DescriptorPlan, powerAttributesFollowConfig) {
    constexpr DeviceConfig K_BUS_POWERED{.vendor_id = 1, .product_id = 1, .self_powered = false, .max_power_ma = 250};
    using BusPowered = DevicePlan<K_BUS_POWERED, Cdc<128, 128>>;
    EXPECT_EQ(BusPowered::K_CONFIGURATION_DESCRIPTOR[7], 0x80);  // bmAttributes: bus powered
    EXPECT_EQ(BusPowered::K_CONFIGURATION_DESCRIPTOR[8], 125);   // bMaxPower in 2 mA units
    EXPECT_EQ(OnePort::K_CONFIGURATION_DESCRIPTOR[7], 0xC0);     // self powered
    EXPECT_EQ(OnePort::K_CONFIGURATION_DESCRIPTOR[8], 50);
}

// Golden vector for one CDC function block at interface 0 with endpoints 0x81 / 0x01 / 0x82 and string 4.
// If this changes, the device will look different to every host: change it on purpose.
TEST(DescriptorPlan, cdcFunctionBlockGoldenBytes) {
    const FunctionLayout layout{
        .first_interface = 0, .first_string_index = 4, .endpoint_addresses = {0x81, 0x01, 0x82, 0}};
    const auto                 block  = Cdc<128, 128>::buildDescriptor(layout);
    const std::vector<uint8_t> golden = {
        0x08, 0x0B, 0x00, 0x02, 0x02, 0x02, 0x01, 0x04,        // IAD
        0x09, 0x04, 0x00, 0x00, 0x01, 0x02, 0x02, 0x01, 0x04,  // communication interface
        0x05, 0x24, 0x00, 0x10, 0x01,                          // header functional
        0x05, 0x24, 0x01, 0x00, 0x01,                          // call management functional
        0x04, 0x24, 0x02, 0x02,                                // ACM functional
        0x05, 0x24, 0x06, 0x00, 0x01,                          // union functional
        0x07, 0x05, 0x81, 0x03, 0x08, 0x00, 0x10,              // notification endpoint
        0x09, 0x04, 0x01, 0x00, 0x02, 0x0A, 0x00, 0x00, 0x00,  // data interface
        0x07, 0x05, 0x01, 0x02, 0x40, 0x00, 0x00,              // data OUT endpoint
        0x07, 0x05, 0x82, 0x02, 0x40, 0x00, 0x00,              // data IN endpoint
    };
    ASSERT_EQ(block.size(), golden.size());
    EXPECT_EQ(std::vector<uint8_t>(block.begin(), block.end()), golden);
}

TEST(DescriptorPlan, twoPortConfigurationHeaderGoldenBytes) {
    const auto&                d             = TwoPorts::K_CONFIGURATION_DESCRIPTOR;
    const std::vector<uint8_t> golden_header = {0x09, 0x02, 0x8D, 0x00, 0x04, 0x01, 0x00, 0xC0, 0x32};
    EXPECT_EQ(std::vector<uint8_t>(d.begin(), d.begin() + 9), golden_header);
    // Second port's IAD starts right after the first port's 66 bytes and names interface 2 and string 5.
    EXPECT_EQ(d[9 + 66 + 0], 0x08);
    EXPECT_EQ(d[9 + 66 + 1], K_DESCRIPTOR_TYPE_INTERFACE_ASSOCIATION);
    EXPECT_EQ(d[9 + 66 + 2], 2);
    EXPECT_EQ(d[9 + 66 + 7], 5);
}
