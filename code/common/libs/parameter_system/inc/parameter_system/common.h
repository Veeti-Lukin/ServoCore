#ifndef COMMON_LIBS_PARAMETERSYSTEM_DEFINITIONS_H
#define COMMON_LIBS_PARAMETERSYSTEM_DEFINITIONS_H

#include <cstdint>
#include <limits>

namespace parameter_system {

using ParameterID                                         = uint8_t;
using ParameterOnChangeCallback                           = void (*)();
constexpr ParameterID                  K_MAX_PARAMETER_ID = std::numeric_limits<ParameterID>::max();

enum class ParameterType : uint8_t {
    saved_parameter,    ///< Device won't change by itself, value saved between the reboots
    runtime_parameter,  ///< Device won't change by itself, value resets between the reboots
    signal,             ///< Only changed by the device, always read only
};

enum class ParameterValueType : uint8_t {
    uint8,
    uint16,
    uint32,
    uint64,
    int8,
    int16,
    int32,
    int64,
    floating_point,
    double_float,

    FIRST_NUMERIC = uint8,
    LAST_NUMERIC  = double_float,

    boolean,

    none,
};

constexpr bool paramTypeIsNumeric(ParameterValueType type) {
    return type >= ParameterValueType::FIRST_NUMERIC && type <= ParameterValueType::LAST_NUMERIC;
}

enum class ReadWriteAccess : uint8_t {
    read_only,
    read_write,
};

enum class ReadWriteResult : uint8_t {
    ok,
    buffer_size_mismatch,
    not_allowed,
};

struct ParameterMetaData {
    static constexpr size_t K_PARAMETER_NAME_MAX_LENGTH = 52;

    ParameterID        id;
    ParameterType      type;
    ParameterValueType value_type;
    ReadWriteAccess    read_write_access;
    char               name[K_PARAMETER_NAME_MAX_LENGTH];
};

inline const char* mapParameterTypeToString(ParameterValueType param_type) {
    switch (param_type) {
        case ParameterValueType::uint8:
            return "uint8";
        case ParameterValueType::uint16:
            return "uint16";
        case ParameterValueType::uint32:
            return "uint32";
        case ParameterValueType::uint64:
            return "uint64";
        case ParameterValueType::int8:
            return "int8";
        case ParameterValueType::int16:
            return "int16";
        case ParameterValueType::int32:
            return "int32";
        case ParameterValueType::int64:
            return "int64";
        case ParameterValueType::floating_point:
            return "floating_point";
        case ParameterValueType::double_float:
            return "double_float";
        case ParameterValueType::boolean:
            return "boolean";
        case ParameterValueType::none:
            return "none";

        default:
            return "unknown";
    }
}

inline const char* mapReadWriteAccessToString(ReadWriteAccess read_write_access) {
    switch (read_write_access) {
        case ReadWriteAccess::read_only:
            return "read_only";
        case ReadWriteAccess::read_write:
            return "read_write";
        default:
            return "unknown";
    }
}

}  // namespace parameter_system

#endif  // COMMON_LIBS_PARAMETERSYSTEM_DEFINITIONS_H
