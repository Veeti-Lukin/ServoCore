#include "protocol_handlers.h"

#include <cstring>

#include "parameter_system/ParameterDatabase.h"
#include "parameter_system/definitions.h"
#include "protocol/requests.h"

extern parameter_system::ParameterDatabase parameter_database;

namespace protocol_handlers {

ResponseData ping(std::span<std::uint8_t> request_data) {
    return {.response_code = serial_communication_framework::ResponseCode::ok, .response_data = {}};
}

ResponseData getParamIds(std::span<std::uint8_t> request_data) {
    protocol::requests::GetRegisteredParameterIds::ResponsePayload response_payload;

    for (size_t i = 0; i < parameter_database.getAmountOfRegisteredParameters(); i++) {
        parameter_system::AbstractParameterDefinition* param_delegate =
            parameter_database.getParameterDefinitionByIndex(i);
        response_payload.registered_parameter_ids.pushBack(param_delegate->getMetaData().id);
    }

    return {.response_code = serial_communication_framework::ResponseCode::ok,
            .response_data = response_payload.serialize()};
}

ResponseData getParamMetaData(std::span<std::uint8_t> request_data) {
    if (request_data.size() != sizeof(parameter_system::ParameterID))
        return {serial_communication_framework::ResponseCode::payload_missing_parts, {}};

    protocol::requests::GetParameterMetaData::RequestPayload request(request_data);

    parameter_system::AbstractParameterDefinition* param_delegate =
        parameter_database.getParameterDefinitionById(request.parameter_id);

    // Parameter not found
    if (param_delegate == nullptr) {
        return {.response_code = serial_communication_framework::ResponseCode::invalid_arguments, .response_data = {}};
    }

    const parameter_system::ParameterMetaData parameter_meta_data = param_delegate->getMetaData();

    return {
        .response_code = serial_communication_framework::ResponseCode::ok,
        .response_data = protocol::requests::GetParameterMetaData::ResponsePayload(parameter_meta_data).serialize()};
}


ResponseData getParamValue(std::span<std::uint8_t> request_data) {
    protocol::requests::ReadParameterValue::RequestPayload request(request_data);

    parameter_system::AbstractParameterDefinition* param = parameter_database.getParameterDefinitionById(request.id);
    // Parameter not found
    if (param == nullptr) {
        return {.response_code = serial_communication_framework::ResponseCode::invalid_arguments, .response_data = {}};
    }

    if (!param->valueIsReadable()) {
        return {serial_communication_framework::ResponseCode::not_allowed, {}};
    }

    if (param->getMetaData().type != request.target_type) {
        return {serial_communication_framework::ResponseCode::type_mismatch, {}};
    }

    protocol::requests::ReadParameterValue::ResponsePayload response;
    param->getValueRaw({response.serialized_value});

    return {serial_communication_framework::ResponseCode::ok, response.serialize()};
}

}  // namespace protocol_handlers
