#ifndef PROTOCOLHANDLER_H
#define PROTOCOLHANDLER_H

#include <cstdint>
#include <span>

#include "drivers/interfaces/BufferedSerialCommunicationInterface.h"
#include "drivers/interfaces/ClockInterface.h"
#include "serial_communication_framework/common.h"
#include "serial_communication_framework/packets.h"

namespace serial_communication_framework {

using OperationCodeHandler = ResponseData (*)(std::span<std::uint8_t> request_data);

struct OperationCodeHandlerInfo {
    uint8_t              operation_code;
    OperationCodeHandler handler;
};

class SlaveHandler {
public:
    explicit SlaveHandler(std::span<OperationCodeHandlerInfo>                        op_code_handler_buffer,
                          drivers::interfaces::BufferedSerialCommunicationInterface& communication_interface,
                          drivers::interfaces::ClockInterface& clock_interface, uint8_t device_id);
    ~        SlaveHandler() = default;

    void init();

    void registerHandler(uint8_t op_code, OperationCodeHandler handler);

    void run();

    [[nodiscard]] const CommunicationStatistics& getCommunicationStatistics() const;

private:
    OperationCodeHandler getOpcodeHandler(uint8_t op_code);

    uint8_t tx_buffer_[ResponsePacket::K_PACKET_MAX_SIZE] = {};
    uint8_t rx_buffer_[RequestPacket::K_PACKET_MAX_SIZE]  = {};

    drivers::interfaces::BufferedSerialCommunicationInterface& communication_interface_;
    CommunicationStatistics                                    communication_statistics_;
    uint8_t                                                    device_id_;

    drivers::interfaces::ClockInterface& timeout_clock_;
    uint64_t                             response_timout_start_time_point_;

    std::span<OperationCodeHandlerInfo> op_code_handlers_;
    size_t                              op_code_handler_registering_index_ = 0;

    void startResponseTimeout();
    bool responseHasTimedout();
};

}  // namespace serial_communication_framework

#endif  // PROTOCOLHANDLER_H
