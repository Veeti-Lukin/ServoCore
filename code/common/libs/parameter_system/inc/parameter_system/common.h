#ifndef COMMON_LIBS_PARAMETERSYSTEM_DEFINITIONS_H
#define COMMON_LIBS_PARAMETERSYSTEM_DEFINITIONS_H

#include <cstddef>
#include <cstdint>
#include <limits>

namespace parameter_system {

using ParameterID                        = uint8_t;
using ParameterOnChangeCallback          = void (*)();
constexpr ParameterID K_MAX_PARAMETER_ID = std::numeric_limits<ParameterID>::max();

enum class ParameterCategory : uint8_t {
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
    not_in_limits,  // Todo numerical limits for numerical parameters
};

struct ParameterMetaData {
    static constexpr size_t K_PARAMETER_NAME_MAX_LENGTH = 52;

    ParameterID        id;
    ParameterCategory  category;
    ParameterValueType value_type;
    ReadWriteAccess    read_write_access;
    char               name[K_PARAMETER_NAME_MAX_LENGTH];
};

}  // namespace parameter_system

#endif  // COMMON_LIBS_PARAMETERSYSTEM_DEFINITIONS_H
