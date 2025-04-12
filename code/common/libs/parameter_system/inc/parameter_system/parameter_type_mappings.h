#ifndef PARAMETER_TYPE_MAPPINGS_H
#define PARAMETER_TYPE_MAPPINGS_H

#include <parameter_system/definitions.h>

#include <cstdint>

namespace parameter_system {
// clang-format off

// ****************************************************************************
// ****** Mapping for each of parameter type to corresponding c++ type ********
// ****************************************************************************

template <ParameterType type>
struct MapParameterTypeToCppType;  // Primary template, left undefined

// Specializations for each supported type

template <> struct MapParameterTypeToCppType<ParameterType::uint8>  { using type = uint8_t; };
template <> struct MapParameterTypeToCppType<ParameterType::uint16> { using type = uint16_t; };
template <> struct MapParameterTypeToCppType<ParameterType::uint32> { using type = uint32_t; };
template <> struct MapParameterTypeToCppType<ParameterType::uint64> { using type = uint64_t; };

template <> struct MapParameterTypeToCppType<ParameterType::int8>   { using type = int8_t; };
template <> struct MapParameterTypeToCppType<ParameterType::int16>  { using type = int16_t; };
template <> struct MapParameterTypeToCppType<ParameterType::int32>  { using type = int32_t; };
template <> struct MapParameterTypeToCppType<ParameterType::int64>  { using type = int64_t; };

template <> struct MapParameterTypeToCppType<ParameterType::floating_point> { using type = float; };
template <> struct MapParameterTypeToCppType<ParameterType::double_float>   { using type = double; };

template <> struct MapParameterTypeToCppType<ParameterType::boolean>        { using type = bool; };

// ****************************************************************************
// ****** Mapping for each of c++ type to corresponding parameter type ********
// ****************************************************************************

template <typename T>
struct MapCppTypeToParameterType;  // Primary template, left undefined

template <> struct MapCppTypeToParameterType<uint8_t>  { static constexpr ParameterType type = ParameterType::uint8; };
template <> struct MapCppTypeToParameterType<uint16_t> { static constexpr ParameterType type = ParameterType::uint16; };
template <> struct MapCppTypeToParameterType<uint32_t> { static constexpr ParameterType type = ParameterType::uint32; };
template <> struct MapCppTypeToParameterType<uint64_t> { static constexpr ParameterType type = ParameterType::uint64; };

template <> struct MapCppTypeToParameterType<int8_t>   { static constexpr ParameterType type = ParameterType::int8; };
template <> struct MapCppTypeToParameterType<int16_t>  { static constexpr ParameterType type = ParameterType::int16; };
template <> struct MapCppTypeToParameterType<int32_t>  { static constexpr ParameterType type = ParameterType::int32; };
template <> struct MapCppTypeToParameterType<int64_t>  { static constexpr ParameterType type = ParameterType::int64; };

template <> struct MapCppTypeToParameterType<float>    { static constexpr ParameterType type = ParameterType::floating_point; };
template <> struct MapCppTypeToParameterType<double>   { static constexpr ParameterType type = ParameterType::double_float; };

template <> struct MapCppTypeToParameterType<bool>     { static constexpr ParameterType type = ParameterType::boolean; };

// clang-format on
}  // namespace parameter_system

#endif  // PARAMETER_TYPE_MAPPINGS_H
