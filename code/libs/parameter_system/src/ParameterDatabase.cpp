#include "parameter_system/ParameterDatabase.h"

#include "assert/assert.h"

namespace parameter_system {
ParameterDatabase::ParameterDatabase(std::span<ParameterDelegateBase*> buffer) : buffer_(buffer) {}

void ParameterDatabase::registerParameter(ParameterDelegateBase* parameter_delegate) {
    ASSERT(parameter_delegate != nullptr);
    ASSERT_WITH_MESSAGE(param_registering_index_ < buffer_.size(),
                        "Parameter registering index out of bounds. Buffer too small");
    ASSERT_WITH_MESSAGE(getParameterDelegateById(parameter_delegate->getMetaData()->id) == nullptr,
                        "Parameter already registered");
    buffer_[param_registering_index_] = parameter_delegate;
    param_registering_index_++;
}

size_t ParameterDatabase::getAmountOfRegisteredParameters() const { return param_registering_index_; }

ParameterDelegateBase* ParameterDatabase::getParameterDelegateById(ParameterID id) const {
    for (ParameterDelegateBase* delegate : buffer_) {
        if (delegate == nullptr) continue;

        if (delegate->getMetaData()->id == id) {
            return delegate;
        }
    }

    return nullptr;
}

ParameterDelegateBase* ParameterDatabase::getParameterDelegateByIndex(size_t index) const {
    if (index >= param_registering_index_ - 1) {
        return nullptr;
    }

    return buffer_[index];
}
std::span<ParameterDelegateBase*> ParameterDatabase::getParameterDelegates() const {
    return buffer_.subspan(0, param_registering_index_);
}

}  // namespace parameter_system