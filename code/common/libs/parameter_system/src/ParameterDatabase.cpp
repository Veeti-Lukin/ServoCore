#include "parameter_system/ParameterDatabase.h"

#include "assert/assert.h"

namespace parameter_system {
ParameterDatabase::ParameterDatabase(std::span<ParameterDefinition*> buffer) : buffer_(buffer) {}

void ParameterDatabase::registerParameter(ParameterDefinition* parameter_definition) {
    ASSERT(parameter_definition != nullptr);
    ASSERT_WITH_MESSAGE(param_registering_index_ < buffer_.size(),
                        "Parameter registering index out of bounds. Buffer too small");
    ASSERT_WITH_MESSAGE(getParameterDefinitionById(parameter_definition->getMetaData().id) == nullptr,
                        "Parameter already registered with the same id");
    buffer_[param_registering_index_] = parameter_definition;
    param_registering_index_++;
}

size_t ParameterDatabase::getAmountOfRegisteredParameters() const { return param_registering_index_; }

ParameterDefinition* ParameterDatabase::getParameterDefinitionById(ParameterID id) const {
    for (ParameterDefinition* definition : buffer_) {
        if (definition == nullptr) continue;

        if (definition->getMetaData().id == id) {
            return definition;
        }
    }

    return nullptr;
}

ParameterDefinition* ParameterDatabase::getParameterDefinitionByIndex(size_t index) const {
    if (index >= param_registering_index_) {
        return nullptr;
    }

    return buffer_[index];
}
std::span<ParameterDefinition*> ParameterDatabase::getParameterDefinitions() const {
    return buffer_.subspan(0, param_registering_index_);
}

}  // namespace parameter_system