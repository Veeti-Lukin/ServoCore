#ifndef COMMON_LIBS_PARAMETERSYSTEM_PARAMETER_DEFINITIONS_H
#define COMMON_LIBS_PARAMETERSYSTEM_PARAMETER_DEFINITIONS_H

#include <cstdint>
#include <cstring>

#include "assert/assert.h"
#include "parameter_system/ParameterDeclaration.h"
#include "parameter_system/definitions.h"
#include "parameter_system/parameter_type_mappings.h"

namespace parameter_system {

using ParameterOnChangeCallback = void (*)();

class AbstractParameterDefinition {
public:
    virtual ~AbstractParameterDefinition() = default;

    [[nodiscard]] ParameterMetaData getMetaData() const { return meta_data_; }

protected:
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

//
//
//
//
//
//

class AbstractNumericParameterDefinition : public AbstractParameterDefinition {
public:
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
    AbstractNumericParameterDefinition(ParameterID id, ReadWriteAccess read_write_access, const char name[],
                                       ParameterType type, void* data_ptr, ParameterOnChangeCallback on_change_cb)
        : AbstractParameterDefinition(id, read_write_access, name, type, data_ptr, on_change_cb) {
        ASSERT_WITH_MESSAGE(paramTypeIsNumeric(type), "Parameter type must be numeric");
    }
};

template <ParameterType T_ParameterType>
class NumericParameterDefinition final : public AbstractNumericParameterDefinition {
public:
    using T = typename MapParameterTypeToCppType<T_ParameterType>::type;

    NumericParameterDefinition(ParameterDeclaration<T_ParameterType> declaration, ReadWriteAccess read_write_access,
                               const char name[], T& data_ref, ParameterOnChangeCallback on_change_cb = nullptr)
        : AbstractNumericParameterDefinition(declaration.id, read_write_access, name, T_ParameterType, &data_ref,
                                             on_change_cb) {}

    ~NumericParameterDefinition() override = default;

    [[nodiscard]] T getValue() const {
        if (meta_data_.read_write_access != ReadWriteAccess::read_only &&
            meta_data_.read_write_access != ReadWriteAccess::read_write) {
            // TODO do what? return 0? assert?
        }

        return deduced_data_ref_;
    }

    void setValue(T value) {
        if (meta_data_.read_write_access != ReadWriteAccess::write_only &&
            meta_data_.read_write_access != ReadWriteAccess::read_write) {
            // TODO do what? return false? assert?
        }

        deduced_data_ref_ = value;
        if (on_change_callback_ != nullptr) {
            on_change_callback_();
        }
    }

    // TODO handle the read write access and on_change_callback
    void setValueFromType(uint8_t value) override { deduced_data_ref_ = value; }
    void setValueFromType(uint16_t value) override { deduced_data_ref_ = value; }
    void setValueFromType(uint32_t value) override { deduced_data_ref_ = value; }
    void setValueFromType(uint64_t value) override { deduced_data_ref_ = value; }
    void setValueFromType(int8_t value) override { deduced_data_ref_ = value; }
    void setValueFromType(int16_t value) override { deduced_data_ref_ = value; }
    void setValueFromType(int32_t value) override { deduced_data_ref_ = value; }
    void setValueFromType(int64_t value) override { deduced_data_ref_ = value; }
    void setValueFromType(float value) override { deduced_data_ref_ = value; }
    void setValueFromType(double value) override { deduced_data_ref_ = value; }

    // TODO handle the read write access
    [[nodiscard]] uint8_t  getValueAsUint8() override { return deduced_data_ref_; }
    [[nodiscard]] uint16_t getValueAsUint16() override { return deduced_data_ref_; }
    [[nodiscard]] uint32_t getValueAsUint32() override { return deduced_data_ref_; }
    [[nodiscard]] uint64_t getValueAsUint64() override { return deduced_data_ref_; }
    [[nodiscard]] int8_t   getValueAsInt8() override { return deduced_data_ref_; }
    [[nodiscard]] int16_t  getValueAsInt16() override { return deduced_data_ref_; }
    [[nodiscard]] int32_t  getValueAsInt32() override { return deduced_data_ref_; }
    [[nodiscard]] int64_t  getValueAsInt64() override { return deduced_data_ref_; }
    [[nodiscard]] float    getValueAsDouble() override { return deduced_data_ref_; }
    [[nodiscard]] double   getValueAsFloat() override { return deduced_data_ref_; }

private:
    T& deduced_data_ref_ = *static_cast<T*>(data_ptr_);
};

//
//
//
//
//
//

class BooleanParameterDefinition final : public AbstractParameterDefinition {
public:
    using T = MapParameterTypeToCppType<ParameterType::boolean>::type;

    BooleanParameterDefinition(ParameterDeclaration<ParameterType::boolean> declaration,
                               ReadWriteAccess read_write_access, const char name[], bool& data_ref,
                               ParameterOnChangeCallback on_change_cb = nullptr)
        : AbstractParameterDefinition(declaration.id, read_write_access, name, ParameterType::boolean, &data_ref,
                                      on_change_cb) {}
    ~BooleanParameterDefinition() override = default;

    [[nodiscard]] T getValue() const {
        if (meta_data_.read_write_access != ReadWriteAccess::read_only &&
            meta_data_.read_write_access != ReadWriteAccess::read_write) {
            // TODO do what? return 0? assert?
        }
        return deduced_data_ref_;
    }
    void setValue(T value) {
        if (meta_data_.read_write_access != ReadWriteAccess::write_only &&
            meta_data_.read_write_access != ReadWriteAccess::read_write) {
            // TODO do what? return false? assert?
        }
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
