#include "parameter_system/ParameterDatabase.h"

#include "assert/assert.h"

namespace parameter_system {
ParameterDatabase::ParameterDatabase(std::span<AbstractParameterDefinition*> buffer) : buffer_(buffer) {}

void ParameterDatabase::registerParameter(AbstractParameterDefinition* parameter_delegate) {
    ASSERT(parameter_delegate != nullptr);
    ASSERT_WITH_MESSAGE(param_registering_index_ < buffer_.size(),
                        "Parameter registering index out of bounds. Buffer too small");
    ASSERT_WITH_MESSAGE(getParameterDelegateById(parameter_delegate->getMetaData().id) == nullptr,
                        "Parameter already registered with the same id");
    buffer_[param_registering_index_] = parameter_delegate;
    param_registering_index_++;
}

size_t ParameterDatabase::getAmountOfRegisteredParameters() const { return param_registering_index_; }

AbstractParameterDefinition* ParameterDatabase::getParameterDelegateById(ParameterID id) const {
    for (AbstractParameterDefinition* delegate : buffer_) {
        if (delegate == nullptr) continue;

        if (delegate->getMetaData().id == id) {
            return delegate;
        }
    }

    return nullptr;
}

AbstractParameterDefinition* ParameterDatabase::getParameterDelegateByIndex(size_t index) const {
    if (index >= param_registering_index_) {
        return nullptr;
    }

    return buffer_[index];
}
std::span<AbstractParameterDefinition*> ParameterDatabase::getParameterDelegates() const {
    return buffer_.subspan(0, param_registering_index_);
}

}  // namespace parameter_system