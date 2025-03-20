add_library(protocol
        inc/protocol/commands.h
        inc/protocol/parameter_ids.h
        inc/protocol/req_types.h
        inc/protocol/resp_types.h
)

set_target_properties(protocol PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(protocol PUBLIC inc)
