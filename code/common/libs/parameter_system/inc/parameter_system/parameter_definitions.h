#ifndef COMMON_LIBS_PARAMETERSYSTEM_PARAMETER_DEFINITIONS_H
#define COMMON_LIBS_PARAMETERSYSTEM_PARAMETER_DEFINITIONS_H

#include <cstdint>
#include <cstring>

#include "assert/assert.h"
#include "parameter_system/ParameterDeclaration.h"
#include "parameter_system/abstract_parameter_definitions.h"
#include "parameter_system/definitions.h"
#include "parameter_system/parameter_type_mappings.h"

namespace parameter_system {

/**
 * @brief Concrete implementation for a numeric parameter.
 * @tparam T_ParameterType The ParameterType (e.g. uint8, int32, float, etc.).
 */
template <ParameterType T_ParameterType>
class NumericParameterDefinition final : public AbstractNumericParameterDefinition {
public:
    using T = typename MapParameterTypeToCppType<T_ParameterType>::type;

    /**
     * @brief Constructs a numeric parameter definition.
     * @param declaration Declaration including the parameter ID.
     * @param read_write_access Read/write access mode.
     * @param name Parameter name.
     * @param data_ref Reference to the data backing this parameter.
     * @param on_change_cb Optional change callback.
     */
    NumericParameterDefinition(ParameterDeclaration<T_ParameterType> declaration, ReadWriteAccess read_write_access,
                               const char name[], T& data_ref, ParameterOnChangeCallback on_change_cb = nullptr)
        : AbstractNumericParameterDefinition(declaration.id, read_write_access, name, T_ParameterType, &data_ref,
                                             on_change_cb) {}

    ~NumericParameterDefinition() override = default;

    /**
     * @brief Retrieves the current value of the parameter.
     *        Checking if the value is readable must be checked explicitly before trying to read the value or this
     *        might assert
     * @return Current value.
     */
    [[nodiscard]] T getValue() const {
        ASSERT_WITH_MESSAGE(valueIsReadable(), "Value is not readable");
        // TODO what to do if the asserts are disabled

        return deduced_data_ref_;
    }

    /**
     * @brief Sets the parameter to a new value and invokes callback if available.
     *        Checking if the value is writable must be checked explicitly before trying to read the value or this
     *        might assert
     * @param value New value.
     */
    void setValue(T value) {
        ASSERT_WITH_MESSAGE(valueIsWritable(), "Value is not writable");
        // TODO what to do if the asserts are disabled

        deduced_data_ref_ = value;
        if (on_change_callback_ != nullptr) {
            on_change_callback_();
        }
    }

    /// Overridden type-agnostic setters.
    void setValueFromType(uint8_t value) override { setValue(value); }
    void setValueFromType(uint16_t value) override { setValue(value); }
    void setValueFromType(uint32_t value) override { setValue(value); }
    void setValueFromType(uint64_t value) override { setValue(value); }
    void setValueFromType(int8_t value) override { setValue(value); }
    void setValueFromType(int16_t value) override { setValue(value); }
    void setValueFromType(int32_t value) override { setValue(value); }
    void setValueFromType(int64_t value) override { setValue(value); }
    void setValueFromType(float value) override { setValue(value); }
    void setValueFromType(double value) override { setValue(value); }

    /// Overridden type-agnostic setters.
    [[nodiscard]] uint8_t  getValueAsUint8() override { return getValue(); }
    [[nodiscard]] uint16_t getValueAsUint16() override { return getValue(); }
    [[nodiscard]] uint32_t getValueAsUint32() override { return getValue(); }
    [[nodiscard]] uint64_t getValueAsUint64() override { return getValue(); }
    [[nodiscard]] int8_t   getValueAsInt8() override { return getValue(); }
    [[nodiscard]] int16_t  getValueAsInt16() override { return getValue(); }
    [[nodiscard]] int32_t  getValueAsInt32() override { return getValue(); }
    [[nodiscard]] int64_t  getValueAsInt64() override { return getValue(); }
    [[nodiscard]] float    getValueAsDouble() override { return getValue(); }
    [[nodiscard]] double   getValueAsFloat() override { return getValue(); }

private:
    T& deduced_data_ref_ = *static_cast<T*>(data_ptr_);
};

//
//
//
//
//
//

/**
 * @brief Concrete implementation for a boolean parameter.
 */
class BooleanParameterDefinition final : public AbstractParameterDefinition {
public:
    using T = MapParameterTypeToCppType<ParameterType::boolean>::type;

    /**
     * @brief Constructs a boolean parameter.
     * @param declaration Parameter declaration.
     * @param read_write_access Read/write access mode.
     * @param name Parameter name.
     * @param data_ref Reference to the boolean data.
     * @param on_change_cb Optional change callback.
     */
    BooleanParameterDefinition(ParameterDeclaration<ParameterType::boolean> declaration,
                               ReadWriteAccess read_write_access, const char name[], bool& data_ref,
                               ParameterOnChangeCallback on_change_cb = nullptr)
        : AbstractParameterDefinition(declaration.id, read_write_access, name, ParameterType::boolean, &data_ref,
                                      on_change_cb) {}
    ~BooleanParameterDefinition() override = default;

    /**
     * @brief Returns the boolean parameter value.
     *        Checking if the value is readable must be checked explicitly before trying to read the value or this
     *        might assert
     * @return Current boolean value.
     */
    [[nodiscard]] T getValue() const {
        ASSERT_WITH_MESSAGE(valueIsReadable(), "Value is not writable");
        // TODO what to do if the asserts are disabled

        return deduced_data_ref_;
    }
    /**
     * @brief Sets the boolean parameter value.
     *        Checking if the value is writable must be checked explicitly before trying to read the value or this
     *        might assert
     * @param value New value to assign.
     */
    void setValue(T value) {
        ASSERT_WITH_MESSAGE(valueIsWritable(), "Value is not writable");
        // TODO what to do if the asserts are disabled

        deduced_data_ref_ = value;
    }

private:
    T& deduced_data_ref_ = *static_cast<T*>(data_ptr_);
};

//
//
//
//
//
//

/// Type aliases for common numeric and boolean parameter types
using ParamUint8   = NumericParameterDefinition<ParameterType::uint8>;
using ParamUint16  = NumericParameterDefinition<ParameterType::uint16>;
using ParamUint32  = NumericParameterDefinition<ParameterType::uint32>;
using ParamUint64  = NumericParameterDefinition<ParameterType::uint64>;

using ParamInt8    = NumericParameterDefinition<ParameterType::int8>;
using ParamInt16   = NumericParameterDefinition<ParameterType::int16>;
using ParamInt32   = NumericParameterDefinition<ParameterType::int32>;
using ParamInt64   = NumericParameterDefinition<ParameterType::int64>;

using ParamFloat   = NumericParameterDefinition<ParameterType::floating_point>;
using ParamDouble  = NumericParameterDefinition<ParameterType::double_float>;

using ParamBoolean = BooleanParameterDefinition;

}  // namespace parameter_system

#endif  // COMMON_LIBS_PARAMETERSYSTEM_PARAMETER_DEFINITIONS_H
