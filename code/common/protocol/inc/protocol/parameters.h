#ifndef COMMON_PROTOCOL_PARAMETERS_H
#define COMMON_PROTOCOL_PARAMETERS_H

#include <cstdint>

#include "parameter_system/ParameterDeclaration.h"
#include "parameter_system/common.h"
#include "protocol/parameter_ids.h"

namespace protocol {

namespace internal {}  // namespace internal

#define DECLARE_PARAMETER(var_name, id, value_type)                                                                \
    static_assert(                                                                                                 \
        (static_cast<ParameterID>(id) <= parameter_system::K_MAX_PARAMETER_ID) && (static_cast<int64_t>(id) >= 0), \
        "Parameter id out of bounds");                                                                             \
    static_assert(true /*TODO*/, "Duplicate ParameterID detected");                                                \
    constexpr ParameterDeclaration<ParameterValueType::value_type>(var_name) { static_cast<ParameterID>(id) }

namespace test_params {
using parameter_system::ParameterDeclaration;
using parameter_system::ParameterID;
using parameter_system::ParameterValueType;

DECLARE_PARAMETER(test_uint8, ParameterIds::test_uint8, uint8);
DECLARE_PARAMETER(test_uint16, ParameterIds::test_uint16, uint16);
DECLARE_PARAMETER(test_uint32, ParameterIds::test_uint32, uint32);
DECLARE_PARAMETER(test_float, ParameterIds::test_float, floating_point);
DECLARE_PARAMETER(test_bool, ParameterIds::test_bool, boolean);
DECLARE_PARAMETER(test_test, 0x99, uint64);

}  // namespace test_params

}  // namespace protocol

#endif  // COMMON_PROTOCOL_PARAMETERS_H
