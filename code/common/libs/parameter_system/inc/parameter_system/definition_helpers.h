#ifndef COMMON_LIBS_PARAMETERSYSTEM_DEFINITION_HELPERS_H
#define COMMON_LIBS_PARAMETERSYSTEM_DEFINITION_HELPERS_H

#include "parameter_system/ParameterDefinition.h"
#include "parameter_system/common.h"

namespace parameter_system {

class SignalParameter final : public ParameterDefinition {
public:
    template <ParameterValueType T_ValueType>
    SignalParameter(const ParameterDeclaration<T_ValueType>& declaration, const char name[],
                    typename MapParameterValueTypeToCppType<T_ValueType>::type& data_ref)
        : ParameterDefinition(declaration, ReadWriteAccess::read_only, name, ParameterCategory::signal, data_ref,
                              nullptr) {}
};

class SavedParameter final : public ParameterDefinition {
public:
    template <ParameterValueType T_ValueType>
    SavedParameter(const ParameterDeclaration<T_ValueType>& declaration, const char name[],
                   typename MapParameterValueTypeToCppType<T_ValueType>::type& data_ref,
                   ParameterOnChangeCallback                                   on_change_callback = nullptr)
        : ParameterDefinition(declaration, ReadWriteAccess::read_write, name, ParameterCategory::saved_parameter,
                              data_ref, on_change_callback) {}
};

class RuntimeParameter : public ParameterDefinition {
public:
    template <ParameterValueType T_ValueType>
    RuntimeParameter(const ParameterDeclaration<T_ValueType>& declaration, const char name[],
                     typename MapParameterValueTypeToCppType<T_ValueType>::type& data_ref,
                     ParameterOnChangeCallback                                   on_change_callback = nullptr)
        : ParameterDefinition(declaration, ReadWriteAccess::read_write, name, ParameterCategory::runtime_parameter,
                              data_ref, on_change_callback) {}
};

}  // namespace parameter_system

#endif  // COMMON_LIBS_PARAMETERSYSTEM_DEFINITION_HELPERS_H
