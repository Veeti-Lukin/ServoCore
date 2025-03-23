#ifndef PARAMETERDELEGATE_H
#define PARAMETERDELEGATE_H

#include <cstdint>

#include "parameter_system/definitions.h"

namespace parameter_system {

using ParameterOnChangeCallback = void (*)();

class ParameterDelegateBase {
public:
    virtual ~ParameterDelegateBase()                                   = default;

    [[nodiscard]] virtual const ParameterMetaData* getMetaData() const = 0;
};

template <typename T, ParameterType parameter_type>
class ParameterDelegate final : public ParameterDelegateBase {
public:
    ParameterDelegate(ParameterID id, ReadWriteAccess read_write_access, const char* name, T& data_ref,
                      ParameterOnChangeCallback callback = nullptr)
        : meta_data_({id, parameter_type, read_write_access, name}),
          data_ref_(data_ref),
          on_change_callback_(callback) {}
    ~ParameterDelegate() override = default;

    T getValue() const {
        if (meta_data_.read_write_access != ReadWriteAccess::read_only &&
            meta_data_.read_write_access != ReadWriteAccess::read_write) {
            // TODO do what? return 0? assert?
        }

        return data_ref_;
    }
    void setValue(T value) {
        if (meta_data_.read_write_access != ReadWriteAccess::write_only &&
            meta_data_.read_write_access != ReadWriteAccess::read_write) {
            // TODO do what? return false? assert?
        }

        data_ref_ = value;
        if (on_change_callback_ != nullptr) {
            on_change_callback_();
        }
    }

    [[nodiscard]] const ParameterMetaData* getMetaData() const override { return &meta_data_; }

private:
    ParameterMetaData         meta_data_;
    T&                        data_ref_;
    ParameterOnChangeCallback on_change_callback_ = nullptr;
};

using ParamUint8   = ParameterDelegate<uint8_t, ParameterType::uint8>;
using ParamUint16  = ParameterDelegate<uint16_t, ParameterType::uint16>;
using ParamUint32  = ParameterDelegate<uint32_t, ParameterType::uint32>;
using ParamUint64  = ParameterDelegate<uint64_t, ParameterType::uint64>;

using ParamInt8    = ParameterDelegate<int8_t, ParameterType::int8>;
using ParamInt16   = ParameterDelegate<int16_t, ParameterType::int16>;
using ParamInt32   = ParameterDelegate<int32_t, ParameterType::int32>;
using ParamInt64   = ParameterDelegate<int64_t, ParameterType::int64>;

using ParamFloat   = ParameterDelegate<float, ParameterType::floating_point>;
using ParamDouble  = ParameterDelegate<double, ParameterType::double_float>;

using ParamBoolean = ParameterDelegate<bool, ParameterType::boolean>;

}  // namespace parameter_system

#endif  // PARAMETERDELEGATE_H
