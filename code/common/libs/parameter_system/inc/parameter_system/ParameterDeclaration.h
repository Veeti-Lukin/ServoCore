#ifndef PARAMETERDECLARATION_H
#define PARAMETERDECLARATION_H

#include "parameter_system/common.h"
#include "parameter_system/parameter_type_mappings.h"

namespace parameter_system {

/**
 * @brief Declares that there is a parameter registered with this id that points to variable of this type
 **/
template <ParameterValueType T_ParameterValueType>
struct ParameterDeclaration {
    ParameterID                         id;
    static constexpr ParameterValueType param_value_type = T_ParameterValueType;
};

}  // namespace parameter_system

#endif  // PARAMETERDECLARATION_H
