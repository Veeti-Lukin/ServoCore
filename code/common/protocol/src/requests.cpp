#include "protocol/requests.h"

#include <cstring>

#include "utils/StaticList.h"

namespace protocol::requests {

GetRegisteredParameterIds::ResponsePayload::ResponsePayload(std::span<uint8_t> buff) {
    for (uint8_t byte : buff) {
        registered_parameter_ids.pushBack(byte);
    }
}
utils::StaticList<uint8_t, K_PAYLOAD_MAX_SIZE> GetRegisteredParameterIds::ResponsePayload::serialize() {
    return registered_parameter_ids;
}
utils::StaticList<uint8_t, K_PAYLOAD_MAX_SIZE> GetParameterMetaData::RequestPayload::serialize() {
    return {parameter_id};
}
GetParameterMetaData::RequestPayload:: RequestPayload(std::span<uint8_t> buffer) { parameter_id = buffer[0]; }
GetParameterMetaData::ResponsePayload::ResponsePayload(std::span<uint8_t> buffer) {
    meta_data                   = {};

    meta_data.id                = buffer[0];
    meta_data.type              = static_cast<parameter_system::ParameterType>(buffer[1]);
    meta_data.read_write_access = static_cast<parameter_system::ReadWriteAccess>(buffer[2]);

    size_t   name_length        = buffer.size_bytes() - 3;
    uint8_t* name_start         = buffer.data() + 3;
    // copy the name to the metadata from the packet
    std::memcpy(meta_data.name, name_start, name_length);
}

utils::StaticList<uint8_t, K_PAYLOAD_MAX_SIZE> GetParameterMetaData::ResponsePayload::serialize() {
    utils::StaticList<uint8_t, K_PAYLOAD_MAX_SIZE> buffer;

    buffer.pushBack(meta_data.id);                                       // byte 1
    buffer.pushBack(static_cast<uint8_t>(meta_data.type));               // byte 2
    buffer.pushBack(static_cast<uint8_t>(meta_data.read_write_access));  // byte 3

    for (const char* it = meta_data.name; *it != '\0'; ++it) {
        buffer.pushBack(*it);
    }
    // add the null termination for the name
    buffer.pushBack('\0');

    return buffer;
}

}  // namespace protocol::requests