#ifndef PARAMETERDECLARATION_H
#define PARAMETERDECLARATION_H

#include "parameter_system/definitions.h"
#include "parameter_system/parameter_type_mappings.h"

namespace parameter_system {

/**
* @brief Declares that there is a parameter registered with this id that points to variable of this type
**/
template <ParameterType T_ParameterType>
struct ParameterDeclaration {
    ParameterID   id;
    static constexpr ParameterType param_type = T_ParameterType;
};

}  // namespace parameter_system

#endif  // PARAMETERDECLARATION_H
