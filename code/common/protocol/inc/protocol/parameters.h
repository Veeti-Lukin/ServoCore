#ifndef COMMON_PROTOCOL_PARAMETERS_H
#define COMMON_PROTOCOL_PARAMETERS_H

#include "parameter_system/ParameterDeclaration.h"
#include "protocol/parameter_ids.h"

namespace protocol {

namespace internal {

// constexpr parameter_system::ParameterDeclaration<> declareParameter() {}
}  // namespace internal

namespace test_params {
using parameter_system::ParameterDeclaration;
using parameter_system::ParameterID;
using parameter_system::ParameterType;

constexpr ParameterDeclaration<ParameterType::uint8>  test_uint8{static_cast<ParameterID>(ParameterIds::test_uint8)};
constexpr ParameterDeclaration<ParameterType::uint16> test_uint16{static_cast<ParameterID>(ParameterIds::test_uint16)};
constexpr ParameterDeclaration<ParameterType::uint32> test_uint32{static_cast<ParameterID>(ParameterIds::test_uint32)};
constexpr ParameterDeclaration<ParameterType::floating_point> test_float{
    static_cast<ParameterID>(ParameterIds::test_float)};
constexpr ParameterDeclaration<ParameterType::boolean> test_bool{static_cast<ParameterID>(ParameterIds::test_bool)};

}  // namespace test_params

}  // namespace protocol

#endif  // COMMON_PROTOCOL_PARAMETERS_H
