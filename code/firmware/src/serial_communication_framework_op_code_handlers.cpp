#include "serial_communication_framework_op_code_handlers.h"

#include <sys/stat.h>

#include <cstring>

#include "parameter_system/ParameterDatabase.h"
#include "parameter_system/definitions.h"

extern parameter_system::ParameterDatabase parameter_database;

namespace serial_communication_framework_op_code_handlers {
ResponseData setParamValue(std::span<std::uint8_t> request_data) { return {}; }

ResponseData getParamValue(std::span<std::uint8_t> request_data) { return {}; }

ResponseData echo(std::span<std::uint8_t> request_data) {
    return {.response_code = serial_communication_framework::ResponseCode::ok, .response_data = request_data};
}

ResponseData ping(std::span<std::uint8_t> request_data) {
    return {.response_code = serial_communication_framework::ResponseCode::ok, .response_data = {}};
}

ResponseData getParamIds(std::span<std::uint8_t> request_data) {
    // TODO should the handlers be given a span to tx buffer for staging the response?;
    // or can they just give response payload as static list?
    static uint8_t buff[255];

    for (size_t i = 0; i < parameter_database.getAmountOfRegisteredParameters(); i++) {
        parameter_system::ParameterDelegateBase* param_delegate = parameter_database.getParameterDelegateByIndex(i);
        buff[i]                                                 = param_delegate->getMetaData()->id;
    }
    return {.response_code = serial_communication_framework::ResponseCode::ok,
            .response_data = {buff, parameter_database.getAmountOfRegisteredParameters()}};
}

ResponseData getParamMetaData(std::span<std::uint8_t> request_data) {
    static uint8_t buff[255];
    if (request_data.size() != sizeof(parameter_system::ParameterID))
        return {serial_communication_framework::ResponseCode::invalid_arguments, {}};

    parameter_system::ParameterID              param_id       = *(request_data.data());
    parameter_system::ParameterDelegateBase*   param_delegate = parameter_database.getParameterDelegateById(param_id);
    const parameter_system::ParameterMetaData* parameter_meta_data = param_delegate->getMetaData();

    buff[0]                                                        = parameter_meta_data->id;
    buff[1]                                                        = static_cast<uint8_t>(parameter_meta_data->type);
    buff[2]  = static_cast<uint8_t>(parameter_meta_data->read_write_access);

    size_t i = 3;
    for (const char* it = parameter_meta_data->name; *it != '\0'; it++) {
        buff[i] = *it;
        i++;
    }
    buff[i] = '\0';
    i++;  // after adding the null termination
    return {.response_code = serial_communication_framework::ResponseCode::ok, .response_data = {buff, i}};
}

}  // namespace serial_communication_framework_op_code_handlers
