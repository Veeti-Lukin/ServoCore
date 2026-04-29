#ifndef PROTOCOLHANDLER_H
#define PROTOCOLHANDLER_H

#include <array>
#include <cstdint>
#include <limits>
#include <span>

#include "debug_print/debug_print.h"
#include "drivers/interfaces/BufferedSerialCommunicationInterface.h"
#include "drivers/interfaces/ClockInterface.h"
#include "serial_communication_framework/command_interface.h"
#include "serial_communication_framework/common.h"
#include "serial_communication_framework/packets.h"

namespace serial_communication_framework {

class SlaveHandler {
public:
    explicit SlaveHandler(drivers::interfaces::BufferedSerialCommunicationInterface& communication_interface,
                          drivers::interfaces::ClockInterface& clock_interface, uint8_t device_id);
    ~SlaveHandler() = default;

    void init();

    template <commands::CommandType T_Command,
              typename T_Command::Response (*T_HandlerFunc)(const typename T_Command::Request&)>
    void registerCommandHandler() {
        ASSERT_WITH_MESSAGE(command_handlers_[T_Command::K_OP_CODE] == nullptr, "Op code already registered");

        // Adapter — static lambda (no captures, convertible to function pointer)
        auto adapter_func = +[](SlaveHandler* self, std::span<std::uint8_t> request_data) -> AdapterFuncResponse {
            typename T_Command::Request  command_req{};
            const commands::ParsingError parse_result = command_req.deserialize(request_data);
            if (parse_result != commands::ParsingError::no_error) {
                DEBUG_PRINT("[SlaveHandler] deserialize failed: op_code=%h payload_size=% parse_error=%\n",
                            T_Command::K_OP_CODE, request_data.size_bytes(), static_cast<uint8_t>(parse_result));
                return {ResponseCode::malformed_request, {}};
            }

            typename T_Command::Response response = T_HandlerFunc(command_req);
            ASSERT_WITH_MESSAGE(response.response_code != ResponseCode::unset_default_value,
                                "Command handler did not set the response code");
            ASSERT_WITH_MESSAGE(response.response_code < ResponseCode::FIRST_INTERNAL ||
                                    response.response_code > ResponseCode::LAST_INTERNAL,
                                "Command handler returned internal error code");

            return {response.response_code, response.serialize(self->command_staging_buffer_)};
        };

        command_handlers_[T_Command::K_OP_CODE] = adapter_func;
    }

    void run();

    [[nodiscard]] const CommunicationStatistics& getCommunicationStatistics() const;

private:
    uint8_t tx_buffer_[ResponsePacket::K_PACKET_MAX_SIZE]              = {};
    uint8_t rx_buffer_[RequestPacket::K_PACKET_MAX_SIZE]               = {};
    uint8_t command_staging_buffer_[RequestPacket::K_PAYLOAD_MAX_SIZE] = {};

    drivers::interfaces::BufferedSerialCommunicationInterface& communication_interface_;
    CommunicationStatistics                                    communication_statistics_;
    uint8_t                                                    device_id_;

    drivers::interfaces::ClockInterface& timeout_clock_;
    uint64_t                             response_timout_start_time_point_;

    struct AdapterFuncResponse {
        ResponseCode            response_code;
        std::span<std::uint8_t> response_data;
    };

    using AdapterFunc = AdapterFuncResponse (*)(SlaveHandler*, std::span<uint8_t>);

    static constexpr size_t K_COMMAND_HANDLER_TABLE_SIZE =
        static_cast<size_t>(std::numeric_limits<decltype(RequestPacket::Header::operation_code)>::max()) + 1;

    std::array<AdapterFunc, K_COMMAND_HANDLER_TABLE_SIZE> command_handlers_ = {};

private:
    void startResponseTimeout();
    bool responseHasTimedout();
};

}  // namespace serial_communication_framework

#endif  // PROTOCOLHANDLER_H
