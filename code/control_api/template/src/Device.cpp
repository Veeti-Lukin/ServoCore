#include "control_api/Device.h"

#include <cstring>

#include "parameter_system/definitions.h"
#include "protocol/requests.h"

namespace servo_core_control_api {
Device::~Device() {}

void Device::executeCommand() {}

uint8_t Device::getId() { return device_id_; }

utils::StaticList<uint8_t, 255> Device::getRegisteredParameterIds() {
    using serial_communication_framework::ResponseCode;
    using serial_communication_framework::ResponseData;

    ResponseData response = communication_handler_->sendRequestAndReceiveResponseBlocking(
        device_id_, protocol::requests::GetRegisteredParameterIds::op_code,
        protocol::requests::GetRegisteredParameterIds::RequestPayload().serialize());

    if (response.response_code != ResponseCode::ok) {
        return {};
    }

    return protocol::requests::GetRegisteredParameterIds::ResponsePayload(response.response_data)
        .registered_parameter_ids;
}

ParameterMetaData Device::getParameterMetaData(uint8_t id) {
    using serial_communication_framework::ResponseCode;
    using serial_communication_framework::ResponseData;

    ResponseData response = communication_handler_->sendRequestAndReceiveResponseBlocking(
        device_id_, protocol::requests::GetParameterMetaData::op_code,
        protocol::requests::GetParameterMetaData::RequestPayload(id).serialize());

    if (response.response_code != ResponseCode::ok) {
        return {};
    }

    return protocol::requests::GetParameterMetaData::ResponsePayload(response.response_data).meta_data;
}

Device::Device(uint8_t id, serial_communication_framework::MasterHandler& communication_handler)
    : device_id_(id), communication_handler_(&communication_handler) {}

}  // namespace servo_core_control_api