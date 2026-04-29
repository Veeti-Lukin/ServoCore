#include "control_api/Device.h"

#include <cstring>

#include "parameter_system/common.h"

namespace servo_core_control_api {
Device::~Device() {}

uint8_t Device::getId() { return device_id_; }

utils::StaticList<ParameterID, parameter_system::K_MAX_PARAMETER_ID> Device::fetchRegisteredParamIds() {
    using serial_communication_framework::ResponseCode;

    protocol::commands::GetRegisteredParamIds::Response response =
        communication_handler_->sendCommandAndReceiveResponseBlocking<protocol::commands::GetRegisteredParamIds>(
            device_id_, {});

    if (response.response_code != ResponseCode::ok) {
        // TODO throw
        return {};
    }

    return response.ids;
}

ParameterMetaData Device::fetchParameterMetaData(ParameterID id) {
    using serial_communication_framework::ResponseCode;

    protocol::commands::GetParamMetadataRequest request;
    request.parameter_id = id;

    protocol::commands::GetParamMetadata::Response response =
        communication_handler_->sendCommandAndReceiveResponseBlocking<protocol::commands::GetParamMetadata>(device_id_,
                                                                                                            request);

    if (response.response_code != ResponseCode::ok) {
        // TODO throw
        return {};
    }

    return response.meta_data;
}

Device::Device(uint8_t id, serial_communication_framework::MasterHandler& communication_handler)
    : device_id_(id), communication_handler_(&communication_handler) {}

}  // namespace servo_core_control_api