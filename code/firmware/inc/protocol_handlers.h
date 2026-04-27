#ifndef serial_communication_framework_OP_CODE_HANDLERS_H
#define serial_communication_framework_OP_CODE_HANDLERS_H

#include "protocol/commands.h"
#include "serial_communication_framework/SlaveHandler.h"

namespace protocol_handlers {

protocol::commands::ReadParamValueResponse   readParamValue(const protocol::commands::ReadParamValueRequest& request);
protocol::commands::EmptyResponse            writeParamValue(const protocol::commands::WriteParamValueRequest& request);
protocol::commands::GetParamMetadataResponse getParamMetaData(
    const protocol::commands::GetParamMetadataRequest& request);

protocol::commands::GetRegisteredParamIdsResponse getParamIds(const protocol::commands::EmptyRequest& request);
protocol::commands::EmptyResponse                 ping(const protocol::commands::EmptyRequest& request);

}  // namespace protocol_handlers

#endif  // serial_communication_framework_OP_CODE_HANDLERS_H
