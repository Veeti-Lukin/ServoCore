#ifndef PARAMETER_SYSTEM_H
#define PARAMETER_SYSTEM_H

#include <span>

#include "parameter_system/definitions.h"
#include "parameter_system/parameter_definitions.h"
#include "parameter_system/parameter_type_mappings.h"

namespace parameter_system {

class ParameterDatabase {
public:
    explicit ParameterDatabase(std::span<AbstractParameterDefinition*> buffer);
    ~        ParameterDatabase() = default;

    void saveParameters() {
        // TODO implement
    }
    void loadParameters() {
        // TODO implement
    }

    void registerParameter(AbstractParameterDefinition* parameter_delegate);

    [[nodiscard]] size_t getAmountOfRegisteredParameters() const;

    [[nodiscard]] AbstractParameterDefinition* getParameterDefinitionById(ParameterID id) const;
    [[nodiscard]] AbstractParameterDefinition* getParameterDefinitionByIndex(size_t index) const;

    template <ParameterType parameter_type>
    [[nodiscard]] auto getParameterDefinitionByIdAs(ParameterID id) const;
    template <ParameterType parameter_type>
    [[nodiscard]] auto getParameterDefinitionByIndexAs(size_t index) const;

    [[nodiscard]] std::span<AbstractParameterDefinition*> getParameterDelegates() const;

private:
    size_t                                  param_registering_index_ = 0;
    std::span<AbstractParameterDefinition*> buffer_;
};

//
//
//
//
//
//

/// ------------------------ TEMPLATE DEFINITIONS --------------------------------------

template <ParameterType T_ParameterType>
auto ParameterDatabase::getParameterDefinitionByIdAs(const ParameterID id) const {
    AbstractParameterDefinition* parameter_definition_ptr = getParameterDefinitionById(id);
    ASSERT(parameter_definition_ptr->getMetaData().type == T_ParameterType);

    // TODO should dynamic cast be used? Only con is that it need RTTI
    if constexpr (T_ParameterType == ParameterType::uint8) {
        return static_cast<ParamUint8*>(parameter_definition_ptr);
    } else if constexpr (T_ParameterType == ParameterType::uint16) {
        return static_cast<ParamUint16*>(parameter_definition_ptr);
    } else if constexpr (T_ParameterType == ParameterType::uint32) {
        return static_cast<ParamUint32*>(parameter_definition_ptr);
    } else if constexpr (T_ParameterType == ParameterType::uint64) {
        return static_cast<ParamUint64*>(parameter_definition_ptr);
    } else if constexpr (T_ParameterType == ParameterType::int8) {
        return static_cast<ParamInt8*>(parameter_definition_ptr);
    } else if constexpr (T_ParameterType == ParameterType::int16) {
        return static_cast<ParamInt16*>(parameter_definition_ptr);
    } else if constexpr (T_ParameterType == ParameterType::int32) {
        return static_cast<ParamInt32*>(parameter_definition_ptr);
    } else if constexpr (T_ParameterType == ParameterType::int64) {
        return static_cast<ParamInt64*>(parameter_definition_ptr);
    } else if constexpr (T_ParameterType == ParameterType::floating_point) {
        return static_cast<ParamFloat*>(parameter_definition_ptr);
    } else if constexpr (T_ParameterType == ParameterType::double_float) {
        return static_cast<ParamDouble*>(parameter_definition_ptr);
    } else if constexpr (T_ParameterType == ParameterType::boolean) {
        return static_cast<ParamBoolean*>(parameter_definition_ptr);
    } else {
        return static_cast<void*>(nullptr);
    }
}

template <ParameterType T_ParameterType>
auto ParameterDatabase::getParameterDefinitionByIndexAs(const size_t index) const {
    AbstractParameterDefinition* parameter_definition_ptr = getParameterDefinitionByIndex(index);
    ASSERT(parameter_definition_ptr->getMetaData().type == T_ParameterType);

    // TODO should dynamic cast be used? Only con is that it need RTTI
    if constexpr (T_ParameterType == ParameterType::uint8) {
        return static_cast<ParamUint8*>(parameter_definition_ptr);
    } else if constexpr (T_ParameterType == ParameterType::uint16) {
        return static_cast<ParamUint16*>(parameter_definition_ptr);
    } else if constexpr (T_ParameterType == ParameterType::uint32) {
        return static_cast<ParamUint32*>(parameter_definition_ptr);
    } else if constexpr (T_ParameterType == ParameterType::uint64) {
        return static_cast<ParamUint64*>(parameter_definition_ptr);
    } else if constexpr (T_ParameterType == ParameterType::int8) {
        return static_cast<ParamInt8*>(parameter_definition_ptr);
    } else if constexpr (T_ParameterType == ParameterType::int16) {
        return static_cast<ParamInt16*>(parameter_definition_ptr);
    } else if constexpr (T_ParameterType == ParameterType::int32) {
        return static_cast<ParamInt32*>(parameter_definition_ptr);
    } else if constexpr (T_ParameterType == ParameterType::int64) {
        return static_cast<ParamInt64*>(parameter_definition_ptr);
    } else if constexpr (T_ParameterType == ParameterType::floating_point) {
        return static_cast<ParamFloat*>(parameter_definition_ptr);
    } else if constexpr (T_ParameterType == ParameterType::double_float) {
        return static_cast<ParamDouble*>(parameter_definition_ptr);
    } else if constexpr (T_ParameterType == ParameterType::boolean) {
        return static_cast<ParamBoolean*>(parameter_definition_ptr);
    } else {
        return static_cast<void*>(nullptr);
    }
}

}  // namespace parameter_system

#endif