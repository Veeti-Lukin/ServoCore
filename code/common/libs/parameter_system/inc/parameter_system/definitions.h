#ifndef COMMON_LIBS_PARAMETERSYSTEM_DEFINITIONS_H
#define COMMON_LIBS_PARAMETERSYSTEM_DEFINITIONS_H

#include <cstdint>
#include <limits>

namespace parameter_system {

using ParameterID                        = uint8_t;
using ParameterOnChangeCallback = void (*)();
constexpr ParameterID K_MAX_PARAMETER_ID = std::numeric_limits<ParameterID>::max();

enum class ParameterType : uint8_t {
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

constexpr bool paramTypeIsNumeric(ParameterType type) {
    return type >= ParameterType::FIRST_NUMERIC && type <= ParameterType::LAST_NUMERIC;
}

enum class ReadWriteAccess : uint8_t {
    read_only,
    write_only,
    read_write,
};

struct ParameterMetaData {
    static constexpr size_t K_PARAMETER_NAME_MAX_LENGTH = 52;

    ParameterID     id;
    ParameterType   type;
    ReadWriteAccess read_write_access;
    char            name[K_PARAMETER_NAME_MAX_LENGTH];
};

inline const char* mapParameterTypeToString(ParameterType param_type) {
    switch (param_type) {
        case ParameterType::uint8:
            return "uint8";
        case ParameterType::uint16:
            return "uint16";
        case ParameterType::uint32:
            return "uint32";
        case ParameterType::uint64:
            return "uint64";
        case ParameterType::int8:
            return "int8";
        case ParameterType::int16:
            return "int16";
        case ParameterType::int32:
            return "int32";
        case ParameterType::int64:
            return "int64";
        case ParameterType::floating_point:
            return "floating_point";
        case ParameterType::double_float:
            return "double_float";
        case ParameterType::boolean:
            return "boolean";
        case ParameterType::none:
            return "none";

        default:
            return "unknown";
    }
}

inline const char* mapReadWriteAccessToString(ReadWriteAccess read_write_access) {
    switch (read_write_access) {
        case ReadWriteAccess::read_only:
            return "read_only";
        case ReadWriteAccess::write_only:
            return "write_only";
        case ReadWriteAccess::read_write:
            return "read_write";
        default:
            return "unknown";
    }
}

}  // namespace parameter_system

#endif  // COMMON_LIBS_PARAMETERSYSTEM_DEFINITIONS_H
