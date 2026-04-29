#include "protocol_handlers.h"

#include <cstring>

#include "parameter_system/ParameterDatabase.h"
#include "parameter_system/common.h"

extern parameter_system::ParameterDatabase parameter_database;

namespace protocol_handlers {

namespace {
bool reboot_pending = false;
}

protocol::commands::ReadParamValueResponse readParamValue(const protocol::commands::ReadParamValueRequest& request) {
    protocol::commands::ReadParamValueResponse response;

    parameter_system::ParameterDefinition* param = parameter_database.getParameterDefinitionById(request.parameter_id);
    // Parameter not found
    if (param == nullptr) {
        response.response_code = serial_communication_framework::ResponseCode::invalid_id;
        return response;
    }

    // Sanity check that type master and firmware has for the parameter is the same. Could drift if master has a stale
    // build of control_api for example
    if (param->getMetaData().value_type != request.expected_value_type) {
        response.response_code = serial_communication_framework::ResponseCode::type_mismatch;
        return response;
    }

    parameter_system::ReadWriteResult reading_result =
        param->getValueRaw({response.raw_bytes, response.K_RAW_BUFF_SIZE}, &response.valid_byte_count);
    if (reading_result != parameter_system::ReadWriteResult::ok) {
        // TODO, probably should handle these somehow
        response.response_code = serial_communication_framework::ResponseCode::unexpected_local_error;
        return response;
    }

    response.response_code = serial_communication_framework::ResponseCode::ok;
    return response;
}

protocol::commands::EmptyResponse writeParamValue(const protocol::commands::WriteParamValueRequest& request) {
    protocol::commands::EmptyResponse response;

    parameter_system::ParameterDefinition* param = parameter_database.getParameterDefinitionById(request.parameter_id);
    // Parameter not found
    if (param == nullptr) {
        response.response_code = serial_communication_framework::ResponseCode::invalid_id;
        return response;
    }

    // Sanity check that type master and firmware has for the parameter is the same. Could drift if master has a stale
    // build of control_api for example
    if (param->getMetaData().value_type != request.expected_value_type) {
        response.response_code = serial_communication_framework::ResponseCode::type_mismatch;
        return response;
    }

    parameter_system::ReadWriteResult writing_result =
        param->setValueRaw({const_cast<uint8_t*>(request.raw_bytes), request.valid_byte_count});

    if (writing_result == parameter_system::ReadWriteResult::not_allowed) {
        response.response_code = serial_communication_framework::ResponseCode::forbidden;
        return response;
    }
    if (writing_result != parameter_system::ReadWriteResult::ok) {
        // TODO, probably should handle these somehow
        response.response_code = serial_communication_framework::ResponseCode::unexpected_local_error;
        return response;
    }

    response.response_code = serial_communication_framework::ResponseCode::ok;
    return response;
}

protocol::commands::GetParamMetadataResponse getParamMetaData(
    const protocol::commands::GetParamMetadataRequest& request) {
    protocol::commands::GetParamMetadataResponse response;

    parameter_system::ParameterDefinition* param = parameter_database.getParameterDefinitionById(request.parameter_id);
    // Parameter not found
    if (param == nullptr) {
        response.response_code = serial_communication_framework::ResponseCode::invalid_id;
        return response;
    }

    response.meta_data     = param->getMetaData();
    response.response_code = serial_communication_framework::ResponseCode::ok;

    return response;
}

protocol::commands::GetRegisteredParamIdsResponse getParamIds(const protocol::commands::EmptyRequest& request) {
    (void)request;  // unused

    protocol::commands::GetRegisteredParamIdsResponse response;

    for (size_t i = 0; i < parameter_database.getAmountOfRegisteredParameters(); i++) {
        ASSERT_WITH_MESSAGE(!response.ids.full(), "Too many registered parameters for response buffer");

        parameter_system::ParameterDefinition* param = parameter_database.getParameterDefinitionByIndex(i);
        response.ids.pushBack(param->getMetaData().id);
    }

    response.response_code = serial_communication_framework::ResponseCode::ok;

    return response;
}

protocol::commands::EmptyResponse ping(const protocol::commands::EmptyRequest& request) {
    (void)request;  // unused

    protocol::commands::EmptyResponse response;
    response.response_code = serial_communication_framework::ResponseCode::ok;
    return response;
}

protocol::commands::EmptyResponse reboot(const protocol::commands::EmptyRequest& request) {
    (void)request;  // unused

    reboot_pending = true;

    protocol::commands::EmptyResponse response;
    response.response_code = serial_communication_framework::ResponseCode::ok;
    return response;
}

bool isRebootPending() { return reboot_pending; }

}  // namespace protocol_handlers
