add_library(parameter_system STATIC
        inc/parameter_system/ParameterDatabase.h
        src/ParameterDatabase.cpp

        inc/parameter_system/ParameterDelegate.h

        inc/parameter_system/parameter_type_mappings.h
)

set_target_properties(parameter_system PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(parameter_system PUBLIC inc)