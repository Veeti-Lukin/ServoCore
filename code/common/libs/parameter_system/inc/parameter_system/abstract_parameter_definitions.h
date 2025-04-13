#ifndef COMMON_LIBS_PARAMETERSYSTEM_ABSTRACT_PARAMETER_DEFINITIONS_H
#define COMMON_LIBS_PARAMETERSYSTEM_ABSTRACT_PARAMETER_DEFINITIONS_H

namespace parameter_system {

/**
 * @brief Base class for all parameter definitions.
 */
class AbstractParameterDefinition {
public:
    virtual ~AbstractParameterDefinition() = default;
    /**
     * @brief Returns the metadata associated with this parameter.
     * @return ParameterMetaData structure containing parameter info.
     */
    [[nodiscard]] ParameterMetaData getMetaData() const { return meta_data_; }

    /**
     * @brief Checks if the parameter value is readable.
     *        Must be checked explicitly before trying to read the value or getters might assert
     * @return True if readable, false otherwise.
     */
    [[nodiscard]] bool valueIsReadable() const {
        return meta_data_.read_write_access == ReadWriteAccess::read_only ||
               meta_data_.read_write_access == ReadWriteAccess::read_write;
    }
    /**
     * @brief Checks if the parameter value is writable.
     *        Must be checked explicitly before trying to write to value or setters might assert
     * @return True if writable, false otherwise.
     */
    [[nodiscard]] bool valueIsWritable() const {
        return meta_data_.read_write_access == ReadWriteAccess::write_only ||
               meta_data_.read_write_access == ReadWriteAccess::read_write;
    }

protected:
    /**
     * @brief Constructor for AbstractParameterDefinition.
     * @param id Unique parameter ID.
     * @param read_write_access Read/write permissions.
     * @param name Name of the parameter (null-terminated string).
     * @param type Parameter type.
     * @param data_ptr Pointer to the actual data.
     * @param on_change_cb Optional callback invoked when the value changes.
     */
    AbstractParameterDefinition(ParameterID id, ReadWriteAccess read_write_access, const char name[],
                                ParameterType type, void* data_ptr, ParameterOnChangeCallback on_change_cb) {
        meta_data_.id                = id;
        meta_data_.read_write_access = read_write_access;
        meta_data_.type              = type;
        // copy the name to the metadata
        for (size_t i = 0; i < ParameterMetaData::K_PARAMETER_NAME_MAX_LENGTH; i++) {
            meta_data_.name[i] = name[i];

            if (name[i] == '\0') break;
            if (i == ParameterMetaData::K_PARAMETER_NAME_MAX_LENGTH - 1)
                ASSERT_WITH_MESSAGE(name[i] == '\0', "Parameter name is too long");
        }

        on_change_callback_ = on_change_cb;
        data_ptr_           = data_ptr;
    }

    ParameterMetaData         meta_data_{};
    ParameterOnChangeCallback on_change_callback_ = nullptr;
    void*                     data_ptr_           = nullptr;
};

/**
 * @brief Abstract class for numeric parameter types.
 */
class AbstractNumericParameterDefinition : public AbstractParameterDefinition {
public:
    /// Setters for various numeric types
    // These methods might cut the actual value when it is set to referenced data
    virtual void setValueFromType(uint8_t value)      = 0;
    virtual void setValueFromType(uint16_t value)     = 0;
    virtual void setValueFromType(uint32_t value)     = 0;
    virtual void setValueFromType(uint64_t value)     = 0;
    virtual void setValueFromType(int8_t value)       = 0;
    virtual void setValueFromType(int16_t value)      = 0;
    virtual void setValueFromType(int32_t value)      = 0;
    virtual void setValueFromType(int64_t value)      = 0;
    virtual void setValueFromType(float value)        = 0;
    virtual void setValueFromType(double value)       = 0;

    /// Getters for various numeric types
    // These methods might return value that is smaller than actually referenced data
    [[nodiscard]] virtual uint8_t  getValueAsUint8()  = 0;
    [[nodiscard]] virtual uint16_t getValueAsUint16() = 0;
    [[nodiscard]] virtual uint32_t getValueAsUint32() = 0;
    [[nodiscard]] virtual uint64_t getValueAsUint64() = 0;
    [[nodiscard]] virtual int8_t   getValueAsInt8()   = 0;
    [[nodiscard]] virtual int16_t  getValueAsInt16()  = 0;
    [[nodiscard]] virtual int32_t  getValueAsInt32()  = 0;
    [[nodiscard]] virtual int64_t  getValueAsInt64()  = 0;
    [[nodiscard]] virtual float    getValueAsDouble() = 0;
    [[nodiscard]] virtual double   getValueAsFloat()  = 0;

protected:
    /**
     * @brief Constructor for numeric parameter definition.
     * @param id Unique parameter ID.
     * @param read_write_access Read/write permissions.
     * @param name Name of the parameter.
     * @param type Parameter type.
     * @param data_ptr Pointer to the actual numeric value.
     * @param on_change_cb Callback on value change.
     */
    AbstractNumericParameterDefinition(ParameterID id, ReadWriteAccess read_write_access, const char name[],
                                       ParameterType type, void* data_ptr, ParameterOnChangeCallback on_change_cb)
        : AbstractParameterDefinition(id, read_write_access, name, type, data_ptr, on_change_cb) {
        ASSERT_WITH_MESSAGE(paramTypeIsNumeric(type), "Parameter type must be numeric");
    }
};

}  // namespace parameter_system

#endif  // COMMON_LIBS_PARAMETERSYSTEM_ABSTRACT_PARAMETER_DEFINITIONS_H
