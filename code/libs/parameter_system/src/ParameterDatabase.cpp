#include "parameter_system/ParameterDatabase.h"

namespace parameter_system {
ParameterDatabase::ParameterDatabase(std::span<ParameterDelegateBase*> buffer) : buffer_(buffer) {}

void ParameterDatabase::registerParameter(ParameterDelegateBase* parameter_delegate) {
    // TODO assert buffer size vs registering index
    // TODO assert id not already registered
    buffer_[param_registering_index_] = parameter_delegate;
    param_registering_index_++;
}

size_t ParameterDatabase::getAmountOfRegisteredParameters() const { return param_registering_index_; }

ParameterDelegateBase* ParameterDatabase::getParameterDelegateById(ParameterID id) const {
    for (ParameterDelegateBase* delegate : buffer_) {
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