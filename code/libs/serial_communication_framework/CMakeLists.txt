add_library(serial_communication_framework STATIC
        inc/serial_communication_framework/common.h

        inc/serial_communication_framework/packets.h

        inc/serial_communication_framework/serialize_deserialize.h
        src/serialize_deserialize.cpp

        inc/serial_communication_framework/SlaveHandler.h
        src/SlaveHandler.cpp
        
)

set_target_properties(serial_communication_framework PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(serial_communication_framework PUBLIC inc)

target_link_libraries(serial_communication_framework PRIVATE drivers_interfaces assert)