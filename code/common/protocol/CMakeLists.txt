add_library(protocol
        inc/protocol/operation_codes.h
        inc/protocol/commands.h
        inc/protocol/parameter_ids.h
        inc/protocol/requests.h
        src/requests.cpp
)

set_target_properties(protocol PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(protocol PUBLIC inc)

target_link_libraries(protocol PUBLIC
        utils
        parameter_system
        serial_communication_framework
)
