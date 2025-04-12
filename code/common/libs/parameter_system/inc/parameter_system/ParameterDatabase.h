#ifndef PARAMETER_SYSTEM_H
#define PARAMETER_SYSTEM_H

#include <span>

#include "parameter_system/ParameterDelegate.h"
#include "parameter_system/definitions.h"
#include "parameter_system/parameter_type_mappings.h"

namespace parameter_system {

class ParameterDatabase {
public:
    explicit ParameterDatabase(std::span<ParameterDelegateBase*> buffer);
    ~        ParameterDatabase() = default;

    void saveParameters() {
        // TODO implement
    }
    void loadParameters() {
        // TODO implement
    }

    void registerParameter(ParameterDelegateBase* parameter_delegate);

    [[nodiscard]] size_t getAmountOfRegisteredParameters() const;

    [[nodiscard]] ParameterDelegateBase* getParameterDelegateById(ParameterID id) const;
    [[nodiscard]] ParameterDelegateBase* getParameterDelegateByIndex(size_t index) const;

    template <ParameterType parameter_type>
    [[nodiscard]] auto getParameterDelegateByIdAs(ParameterID id) const;
    template <ParameterType parameter_type>
    [[nodiscard]] auto getParameterDelegateByIndexAs(size_t index) const;

    [[nodiscard]] std::span<ParameterDelegateBase*> getParameterDelegates() const;

private:
    size_t                            param_registering_index_ = 0;
    std::span<ParameterDelegateBase*> buffer_;
};

//
//
//
//
//
//

/// ------------------------ TEMPLATE DEFINITIONS --------------------------------------

template <ParameterType parameter_type>
auto ParameterDatabase::getParameterDelegateByIdAs(const ParameterID id) const {
    ParameterDelegateBase* parameter_delegate = getParameterDelegateById(id);

    // Check if the stored type matches the requested type
    if (parameter_delegate->getMetaData()->type == parameter_type) {
        return static_cast<ParameterDelegate<typename MapParameterTypeToCppType<parameter_type>::type, parameter_type>*>(
            parameter_delegate);
    }

    return static_cast<ParameterDelegate<typename MapParameterTypeToCppType<parameter_type>::type, parameter_type>*>(
        nullptr);
}

template <ParameterType parameter_type>
auto ParameterDatabase::getParameterDelegateByIndexAs(const size_t index) const {
    ParameterDelegateBase* parameter_delegate = getParameterDelegateByIndex(index);

    // Check if the stored type matches the requested type
    if (parameter_delegate->getMetaData()->type == parameter_type) {
        return static_cast<ParameterDelegate<typename MapParameterTypeToCppType<parameter_type>::type, parameter_type>*>(
            parameter_delegate);
    }

    return static_cast<ParameterDelegate<typename MapParameterTypeToCppType<parameter_type>::type, parameter_type>*>(
        nullptr);
}

}  // namespace parameter_system

#endif