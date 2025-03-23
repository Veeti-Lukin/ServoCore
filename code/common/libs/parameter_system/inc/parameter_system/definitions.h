#ifndef COMMON_LIBS_PARAMETERSYSTEM_DEFINITIONS_H
#define COMMON_LIBS_PARAMETERSYSTEM_DEFINITIONS_H

#include <limits>

namespace parameter_system {

using ParameterID                        = uint8_t;
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
    boolean,

    none,
};

enum class ReadWriteAccess : uint8_t {
    read_only,
    write_only,
    read_write,
};

struct ParameterMetaData {
    ParameterID     id;
    ParameterType   type;
    ReadWriteAccess read_write_access;
    const char*     name;
};

}  // namespace parameter_system

#endif  // COMMON_LIBS_PARAMETERSYSTEM_DEFINITIONS_H
