#ifndef COMMON_LIBS_PARAMETERSYSTEM_PARAMETERDEFINITION_H
#define COMMON_LIBS_PARAMETERSYSTEM_PARAMETERDEFINITION_H

#include <cstring>
#include <span>

#include "assert/assert.h"
#include "parameter_system/ParameterDeclaration.h"
#include "parameter_system/common.h"
#include "parameter_type_mappings.h"

namespace parameter_system {

/**
 * @brief A Parameter definiton used to define and register a parameter on a firmware level.
 */
class ParameterDefinition {
public:
    /**
     * @brief Constructs a ParameterDefinition from a ParameterDeclaration.
     *
     * @tparam T_ValueType  The declared parameter value type.
     * @param declaration   The corresponding parameter declaration.
     * @param read_write_access Read/write access mode.
     * @param name          Null-terminated parameter name.
     * @param type          Logical parameter type.
     * @param data_ref      Reference to the parameter’s underlying variable.
     * @param on_change_cb  Optional callback invoked when the value changes.
     */
    template <ParameterValueType T_ValueType>
    ParameterDefinition(const ParameterDeclaration<T_ValueType>& declaration, ReadWriteAccess read_write_access,
                        const char name[], ParameterType type,
                        typename MapParameterValueTypeToCppType<T_ValueType>::type& data_ref,
                        ParameterOnChangeCallback                                   on_change_cb) {
        meta_data_.id                = declaration.id;
        meta_data_.read_write_access = read_write_access;
        meta_data_.type              = type;
        meta_data_.value_type        = declaration.param_value_type;
        // copy the name to the metadata
        for (size_t i = 0; i < ParameterMetaData::K_PARAMETER_NAME_MAX_LENGTH; i++) {
            meta_data_.name[i] = name[i];

            if (name[i] == '\0') break;
            if (i == ParameterMetaData::K_PARAMETER_NAME_MAX_LENGTH - 1)
                ASSERT_WITH_MESSAGE(name[i] == '\0', "Parameter name is too long");
        }

        on_change_callback_ = on_change_cb;
        data_ptr_           = &data_ref;
        pointed_data_size_  = sizeOfCppTypeByParameterValueType(declaration.param_value_type);
    }

    virtual ~ParameterDefinition() = default;

    /**
     * @brief Retrieves metadata describing this parameter.
     *
     * @return A copy of the parameter's metadata structure.
     */
    [[nodiscard]] ParameterMetaData getMetaData() const { return meta_data_; }

    /**
     * @brief Checks whether the parameter can be written to.
     *
     * @return True if writable, false if read-only.
     */
    [[nodiscard]] bool valueIsWritable() const { return meta_data_.read_write_access == ReadWriteAccess::read_write; }

    /**
     * @brief Sets the parameter’s value from a raw byte buffer.
     *
     * Parses the provided buffer and writes the value to the underlying variable.
     * If the parameter is not writable, this function returns a failure result.
     *
     * @param buff Byte buffer containing the new parameter value.
     * @return ReadWriteResult::ok if the value was successfully written,
     *         otherwise ReadWriteResult::not_allowed or ReadWriteResult::buffer_size_mismatch.
     */
    [[nodiscard]] ReadWriteResult setValueRaw(std::span<uint8_t> buff);

    /**
     * @brief Serializes the parameter’s current value into a raw byte buffer.
     *
     * Copies the binary representation of the underlying variable into @p target_buff.
     * The buffer must be large enough to hold the entire value.
     *
     * @param target_buff Output buffer to receive the serialized value.
     * @param written_bytes_out Optional output. If non-null, set to the number of bytes written into @p target_buff.
     * @return ReadWriteResult::ok if successful, otherwise ReadWriteResult::error.
     */
    [[nodiscard]] ReadWriteResult getValueRaw(std::span<uint8_t> target_buff, size_t* written_bytes_out = nullptr);

protected:
    ParameterMetaData         meta_data_{};
    ParameterOnChangeCallback on_change_callback_ = nullptr;
    void*                     data_ptr_ = nullptr;  // < points to a stack allocated object, no any kind of ownership
    size_t                    pointed_data_size_ = 0;
};

}  // namespace parameter_system

#endif  // COMMON_LIBS_PARAMETERSYSTEM_PARAMETERDEFINITION_H
