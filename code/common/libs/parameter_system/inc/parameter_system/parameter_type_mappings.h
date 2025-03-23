#ifndef PARAMETER_TYPE_MAPPINGS_H
#define PARAMETER_TYPE_MAPPINGS_H

#include "parameter_system/ParameterDelegate.h"

namespace parameter_system {

// Mapping for each of parameter type to c++ type

template <ParameterType type>
struct ParameterTypeMapping;  // Primary template, left undefined

// Specializations for each supported type
// clang-format off
template <> struct ParameterTypeMapping<ParameterType::uint8>  { using type = uint8_t; };
template <> struct ParameterTypeMapping<ParameterType::uint16> { using type = uint16_t; };
template <> struct ParameterTypeMapping<ParameterType::uint32> { using type = uint32_t; };
template <> struct ParameterTypeMapping<ParameterType::uint64> { using type = uint64_t; };
template <> struct ParameterTypeMapping<ParameterType::int8>   { using type = int8_t; };
template <> struct ParameterTypeMapping<ParameterType::int16>  { using type = int16_t; };
template <> struct ParameterTypeMapping<ParameterType::int32>  { using type = int32_t; };
template <> struct ParameterTypeMapping<ParameterType::int64>  { using type = int64_t; };
template <> struct ParameterTypeMapping<ParameterType::floating_point> { using type = float; };
template <> struct ParameterTypeMapping<ParameterType::double_float>   { using type = double; };
template <> struct ParameterTypeMapping<ParameterType::boolean>   { using type = bool; };
// clang-format on

}  // namespace parameter_system

#endif  // PARAMETER_TYPE_MAPPINGS_H
