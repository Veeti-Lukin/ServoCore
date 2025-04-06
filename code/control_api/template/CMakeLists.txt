add_library(control_api_template
        inc/control_api/Device.h
        src/Device.cpp

        inc/control_api/Context.h
        src/Context.cpp
)

set_target_properties(control_api_template PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(control_api_template PUBLIC inc)

target_link_libraries(control_api_template PUBLIC
        drivers_interfaces
        serial_communication_framework
        protocol
        utils
        parameter_system
)
