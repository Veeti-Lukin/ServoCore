#include <gtest/gtest.h>

#include <cstdint>
#include <deque>
#include <span>
#include <vector>

#include "drivers/interfaces/BufferedSerialCommunicationInterface.h"
#include "drivers/interfaces/ClockInterface.h"
#include "math/crc.h"
#include "serial_communication_framework/SlaveHandler.h"
#include "serial_communication_framework/command_interface.h"
#include "serial_communication_framework/common.h"
#include "serial_communication_framework/packets.h"

namespace {

using serial_communication_framework::RequestPacket;
using serial_communication_framework::ResponseCode;
using serial_communication_framework::SlaveHandler;

/** @brief A link the test can push request bytes into and read whatever the handler answered. */
class FakeLink final : public drivers::interfaces::BufferedSerialCommunicationInterface {
public:
    void transmitByte(uint8_t byte) override { transmitted.push_back(byte); }

    void transmitBytes(std::span<uint8_t> bytes) override {
        for (const uint8_t byte : bytes) {
            transmitted.push_back(byte);
        }
    }

    size_t getReceivedBytesAvailableAmount() override { return to_receive.size(); }

    uint8_t readReceivedByte() override {
        const uint8_t byte = to_receive.front();
        to_receive.pop_front();
        return byte;
    }

    size_t readReceivedBytes(std::span<uint8_t> bytes) override {
        size_t count = 0;
        while (count < bytes.size() && !to_receive.empty()) {
            bytes[count++] = readReceivedByte();
        }
        return count;
    }

    void push(const std::vector<uint8_t>& bytes) { to_receive.insert(to_receive.end(), bytes.begin(), bytes.end()); }

    std::deque<uint8_t>  to_receive;
    std::vector<uint8_t> transmitted;
};

/** @brief Always reports zero, so the slave-side response timeout never fires during a test. */
class FakeClock final : public drivers::interfaces::ClockInterface {
public:
    uint64_t uptimeMicroseconds() override { return 0; }
    uint64_t uptimeMilliseconds() override { return 0; }
    uint64_t uptimeSeconds() override { return 0; }
};

constexpr uint8_t K_TEST_OP_CODE = 0x01;

using TestCommand = serial_communication_framework::commands::Command<
    serial_communication_framework::commands::EmptyRequest,
    serial_communication_framework::commands::EmptyResponse, K_TEST_OP_CODE>;

serial_communication_framework::commands::EmptyResponse testHandler(
    const serial_communication_framework::commands::EmptyRequest& request) {
    (void)request;
    serial_communication_framework::commands::EmptyResponse response;
    response.response_code = ResponseCode::ok;
    return response;
}

std::vector<uint8_t> makeRequest(uint8_t receiver_id, uint8_t op_code) {
    std::vector<uint8_t> header{receiver_id, op_code, 0};
    const uint8_t        header_crc  = math::generateCrc8(std::span<uint8_t>(header));
    const uint8_t        payload_crc = math::generateCrc8(std::span<uint8_t>());
    return {receiver_id, op_code, 0, header_crc, payload_crc};
}

/** @brief run() consumes one byte per call, so a packet needs several turns of the caller's loop. */
void pumpInterleaved(SlaveHandler& a, SlaveHandler& b, int iterations) {
    for (int i = 0; i < iterations; i++) {
        a.run();
        b.run();
    }
}

TEST(SlaveHandlerReceiveState, answersARequestOnASingleLink) {
    FakeLink     link;
    FakeClock    clock;
    SlaveHandler handler(link, clock, 0);
    handler.registerCommandHandler<TestCommand, testHandler>();

    link.push(makeRequest(0, K_TEST_OP_CODE));
    for (int i = 0; i < 16; i++) {
        handler.run();
    }

    ASSERT_GE(link.transmitted.size(), 4u);
    EXPECT_EQ(link.transmitted[0], static_cast<uint8_t>(ResponseCode::ok));
    EXPECT_EQ(link.transmitted[1], 0);  // no payload
}

// A device that serves the protocol on more than one link (field bus UART and USB) runs one handler per link
// from the same loop. The receive progress of a half-assembled packet therefore has to belong to the handler:
// when it was shared, whichever handler reached K_HEADER_SIZE first parsed the *other* one's buffer, failed the
// CRC and reset the shared index, so neither packet ever completed.
TEST(SlaveHandlerReceiveState, twoHandlersInterleavedDoNotDisturbEachOther) {
    FakeLink  link_a;
    FakeLink  link_b;
    FakeClock clock;

    SlaveHandler handler_a(link_a, clock, 0);
    SlaveHandler handler_b(link_b, clock, 0);
    handler_a.registerCommandHandler<TestCommand, testHandler>();
    handler_b.registerCommandHandler<TestCommand, testHandler>();

    link_a.push(makeRequest(0, K_TEST_OP_CODE));
    link_b.push(makeRequest(0, K_TEST_OP_CODE));

    pumpInterleaved(handler_a, handler_b, 16);

    ASSERT_GE(link_a.transmitted.size(), 4u) << "handler A never answered";
    ASSERT_GE(link_b.transmitted.size(), 4u) << "handler B never answered";
    EXPECT_EQ(link_a.transmitted[0], static_cast<uint8_t>(ResponseCode::ok));
    EXPECT_EQ(link_b.transmitted[0], static_cast<uint8_t>(ResponseCode::ok));

    EXPECT_EQ(handler_a.getCommunicationStatistics().valid_packets_received, 1u);
    EXPECT_EQ(handler_b.getCommunicationStatistics().valid_packets_received, 1u);
    EXPECT_EQ(handler_a.getCommunicationStatistics().corrupted_packets_received, 0u);
    EXPECT_EQ(handler_b.getCommunicationStatistics().corrupted_packets_received, 0u);
}

// The links are driven at different rates here, so the two handlers sit at different offsets inside their
// packets - the case that made the shared index fail intermittently rather than every time.
TEST(SlaveHandlerReceiveState, twoHandlersAtDifferentPacketOffsets) {
    FakeLink  link_a;
    FakeLink  link_b;
    FakeClock clock;

    SlaveHandler handler_a(link_a, clock, 0);
    SlaveHandler handler_b(link_b, clock, 0);
    handler_a.registerCommandHandler<TestCommand, testHandler>();
    handler_b.registerCommandHandler<TestCommand, testHandler>();

    link_a.push(makeRequest(0, K_TEST_OP_CODE));
    // Let A get part way into its packet before B's request even starts arriving.
    for (int i = 0; i < 3; i++) {
        handler_a.run();
        handler_b.run();
    }
    link_b.push(makeRequest(0, K_TEST_OP_CODE));
    pumpInterleaved(handler_a, handler_b, 16);

    ASSERT_GE(link_a.transmitted.size(), 4u) << "handler A never answered";
    ASSERT_GE(link_b.transmitted.size(), 4u) << "handler B never answered";
    EXPECT_EQ(link_a.transmitted[0], static_cast<uint8_t>(ResponseCode::ok));
    EXPECT_EQ(link_b.transmitted[0], static_cast<uint8_t>(ResponseCode::ok));
}

// Back-to-back requests on both links: the handlers must stay framed across packet boundaries.
TEST(SlaveHandlerReceiveState, repeatedRequestsStayFramed) {
    FakeLink  link_a;
    FakeLink  link_b;
    FakeClock clock;

    SlaveHandler handler_a(link_a, clock, 0);
    SlaveHandler handler_b(link_b, clock, 0);
    handler_a.registerCommandHandler<TestCommand, testHandler>();
    handler_b.registerCommandHandler<TestCommand, testHandler>();

    constexpr int K_REQUESTS = 8;
    for (int i = 0; i < K_REQUESTS; i++) {
        link_a.push(makeRequest(0, K_TEST_OP_CODE));
        link_b.push(makeRequest(0, K_TEST_OP_CODE));
    }

    pumpInterleaved(handler_a, handler_b, K_REQUESTS * RequestPacket::K_PACKET_MIN_SIZE + 16);

    EXPECT_EQ(handler_a.getCommunicationStatistics().valid_packets_received, K_REQUESTS);
    EXPECT_EQ(handler_b.getCommunicationStatistics().valid_packets_received, K_REQUESTS);
    EXPECT_EQ(handler_a.getCommunicationStatistics().corrupted_packets_received, 0u);
    EXPECT_EQ(handler_b.getCommunicationStatistics().corrupted_packets_received, 0u);
    EXPECT_EQ(link_a.transmitted.size(), K_REQUESTS * 4u);
    EXPECT_EQ(link_b.transmitted.size(), K_REQUESTS * 4u);
}

}  // namespace
