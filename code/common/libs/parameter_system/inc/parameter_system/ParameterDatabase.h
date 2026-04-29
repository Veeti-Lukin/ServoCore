#ifndef PARAMETER_SYSTEM_H
#define PARAMETER_SYSTEM_H

#include <span>

#include "parameter_system/ParameterDefinition.h"
#include "parameter_system/common.h"
#include "parameter_system/parameter_type_mappings.h"

namespace parameter_system {

class ParameterDatabase {
public:
    explicit ParameterDatabase(std::span<ParameterDefinition*> buffer);
    ~        ParameterDatabase() = default;

    void saveParameters() {
        // TODO implement
    }
    void loadParameters() {
        // TODO implement
    }

    void registerParameter(ParameterDefinition* parameter_definition);

    [[nodiscard]] size_t getAmountOfRegisteredParameters() const;

    [[nodiscard]] ParameterDefinition* getParameterDefinitionById(ParameterID id) const;
    [[nodiscard]] ParameterDefinition* getParameterDefinitionByIndex(size_t index) const;

    [[nodiscard]] std::span<ParameterDefinition*> getParameterDefinitions() const;

private:
    size_t                                  param_registering_index_ = 0;
    std::span<ParameterDefinition*> buffer_;
};

}  // namespace parameter_system

#endif