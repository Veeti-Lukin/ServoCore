add_library(protocol
        inc/protocol/parameter_ids.h
        inc/protocol/parameters.h

        # -------- commands --------
        inc/protocol/commands.h
        inc/protocol/commands/internal/op_codes.h

        inc/protocol/commands/ping_command.h

        inc/protocol/commands/get_registered_param_ids_command.h
        src/commands/get_registered_param_ids_command.cpp

        inc/protocol/commands/get_param_metadata_command.h
        src/commands/get_param_metadata_command.cpp

        inc/protocol/commands/read_parm_value_command.h
        src/commands/read_param_value_command.cpp

        inc/protocol/commands/write_param_value_command.h
        src/commands/write_param_value_command.cpp
)

set_target_properties(protocol PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(protocol PUBLIC inc)

target_link_libraries(protocol PUBLIC
        utils
        parameter_system
        serial_communication_framework
)
