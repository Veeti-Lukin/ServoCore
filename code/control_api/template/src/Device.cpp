#include "control_api/Device.h"

#include <cstring>

#include "parameter_system/definitions.h"
#include "protocol/commands.h"

namespace servo_core_control_api {
Device::~Device() {}

void Device::executeCommand() {}

uint8_t Device::getId() { return device_id_; }

utils::StaticList<uint8_t, 255> Device::getRegisteredParameterIds() {
    using serial_communication_framework::ResponseCode;
    using serial_communication_framework::ResponseData;

    ResponseData response = communication_handler_->sendRequestAndReceiveResponseBlocking(
        device_id_, static_cast<uint8_t>(protocol::ParameterCommands::get_ids), {});

    if (response.response_code != ResponseCode::ok) {
        return {};
    }

    utils::StaticList<uint8_t, 255> ids;
    for (uint8_t id : response.response_data) {
        ids.pushBack(id);
    }
    return ids;
}

ParameterMetaData Device::getParameterMetaData(uint8_t id) {
    using serial_communication_framework::ResponseCode;
    using serial_communication_framework::ResponseData;

    ResponseData response = communication_handler_->sendRequestAndReceiveResponseBlocking(
        device_id_, static_cast<uint8_t>(protocol::ParameterCommands::get_metadata), {&id, 1});
    if (response.response_code != ResponseCode::ok) {
        return {};
    }

    ParameterMetaData meta_data;
    meta_data.id                = response.response_data[0];
    meta_data.type              = static_cast<parameter_system::ParameterType>(response.response_data[1]);
    meta_data.read_write_access = static_cast<parameter_system::ReadWriteAccess>(response.response_data[2]);

    size_t   name_length        = response.response_data.size_bytes() - 3;
    uint8_t* name_start         = response.response_data.data() + 3;
    // copy the name to the metadata from the packet
    std::memcpy(meta_data.name, name_start, name_length);

    return meta_data;
}

Device::Device(uint8_t id, serial_communication_framework::MasterHandler& communication_handler)
    : device_id_(id), communication_handler_(&communication_handler) {}

}  // namespace servo_core_control_api