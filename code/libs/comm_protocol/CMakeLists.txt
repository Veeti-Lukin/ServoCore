add_library(comm_protocol STATIC
        inc/comm_protocol/packets.h

        inc/comm_protocol/serialize_deserialize.h
        src/serialize_deserialize.cpp

        inc/comm_protocol/SlaveProtocolHandler.h
        src/SlaveProtocolHandler.cpp
)

set_target_properties(comm_protocol PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(comm_protocol PUBLIC inc)

target_link_libraries(comm_protocol PRIVATE drivers)