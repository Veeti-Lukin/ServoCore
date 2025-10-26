#ifndef PARAMETER_TYPE_MAPPINGS_H
#define PARAMETER_TYPE_MAPPINGS_H

#include <parameter_system/common.h>

#include <cstdint>

#include "assert/assert.h"

namespace parameter_system {
// clang-format off

// ****************************************************************************
// ****** Mapping for each of parameter type to corresponding c++ type ********
// ****************************************************************************

template <ParameterValueType type>
struct MapParameterValueTypeToCppType;  // Primary template, left undefined

// Specializations for each supported type

template <> struct MapParameterValueTypeToCppType<ParameterValueType::uint8>  { using type = uint8_t; };
template <> struct MapParameterValueTypeToCppType<ParameterValueType::uint16> { using type = uint16_t; };
template <> struct MapParameterValueTypeToCppType<ParameterValueType::uint32> { using type = uint32_t; };
template <> struct MapParameterValueTypeToCppType<ParameterValueType::uint64> { using type = uint64_t; };

template <> struct MapParameterValueTypeToCppType<ParameterValueType::int8>   { using type = int8_t; };
template <> struct MapParameterValueTypeToCppType<ParameterValueType::int16>  { using type = int16_t; };
template <> struct MapParameterValueTypeToCppType<ParameterValueType::int32>  { using type = int32_t; };
template <> struct MapParameterValueTypeToCppType<ParameterValueType::int64>  { using type = int64_t; };

template <> struct MapParameterValueTypeToCppType<ParameterValueType::floating_point> { using type = float; };
template <> struct MapParameterValueTypeToCppType<ParameterValueType::double_float>   { using type = double; };

template <> struct MapParameterValueTypeToCppType<ParameterValueType::boolean>        { using type = bool; };

// ****************************************************************************
// ****** Mapping for each of c++ type to corresponding parameter type ********
// ****************************************************************************

template <typename T>
struct MapCppTypeToParameterValueType;  // Primary template, left undefined

template <> struct MapCppTypeToParameterValueType<uint8_t>  { static constexpr ParameterValueType type = ParameterValueType::uint8; };
template <> struct MapCppTypeToParameterValueType<uint16_t> { static constexpr ParameterValueType type = ParameterValueType::uint16; };
template <> struct MapCppTypeToParameterValueType<uint32_t> { static constexpr ParameterValueType type = ParameterValueType::uint32; };
template <> struct MapCppTypeToParameterValueType<uint64_t> { static constexpr ParameterValueType type = ParameterValueType::uint64; };

template <> struct MapCppTypeToParameterValueType<int8_t>   { static constexpr ParameterValueType type = ParameterValueType::int8; };
template <> struct MapCppTypeToParameterValueType<int16_t>  { static constexpr ParameterValueType type = ParameterValueType::int16; };
template <> struct MapCppTypeToParameterValueType<int32_t>  { static constexpr ParameterValueType type = ParameterValueType::int32; };
template <> struct MapCppTypeToParameterValueType<int64_t>  { static constexpr ParameterValueType type = ParameterValueType::int64; };

template <> struct MapCppTypeToParameterValueType<float>    { static constexpr ParameterValueType type = ParameterValueType::floating_point; };
template <> struct MapCppTypeToParameterValueType<double>   { static constexpr ParameterValueType type = ParameterValueType::double_float; };

template <> struct MapCppTypeToParameterValueType<bool>     { static constexpr ParameterValueType type = ParameterValueType::boolean; };



inline size_t sizeOfCppTypeByParameterValueType(ParameterValueType type) {
    switch (type) {
        case ParameterValueType::uint8:
            return sizeof(MapParameterValueTypeToCppType<ParameterValueType::uint8>::type);
        case ParameterValueType::uint16:
            return sizeof(MapParameterValueTypeToCppType<ParameterValueType::uint16>::type);
        case ParameterValueType::uint32:
            return sizeof(MapParameterValueTypeToCppType<ParameterValueType::uint8>::type);
        case ParameterValueType::uint64:
            return sizeof(MapParameterValueTypeToCppType<ParameterValueType::uint8>::type);
        case ParameterValueType::int8:
            return sizeof(MapParameterValueTypeToCppType<ParameterValueType::uint8>::type);
        case ParameterValueType::int16:
            return sizeof(MapParameterValueTypeToCppType<ParameterValueType::uint8>::type);
        case ParameterValueType::int32:
            return sizeof(MapParameterValueTypeToCppType<ParameterValueType::uint8>::type);
        case ParameterValueType::int64:
            return sizeof(MapParameterValueTypeToCppType<ParameterValueType::uint8>::type);
        case ParameterValueType::floating_point:
            return sizeof(MapParameterValueTypeToCppType<ParameterValueType::uint8>::type);
        case ParameterValueType::double_float:
            return sizeof(MapParameterValueTypeToCppType<ParameterValueType::uint8>::type);
        case ParameterValueType::boolean:
            return sizeof(MapParameterValueTypeToCppType<ParameterValueType::uint8>::type);
        case ParameterValueType::none:
            return sizeof(MapParameterValueTypeToCppType<ParameterValueType::uint8>::type);
    }
    ASSERT_WITH_MESSAGE(false, "Unhandled map parameter type");
    return 0;
}
// clang-format on
}  // namespace parameter_system

#endif  // PARAMETER_TYPE_MAPPINGS_H
