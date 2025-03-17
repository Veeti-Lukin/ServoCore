add_library(drivers_interfaces
        inc/drivers/interfaces/SerialBufferedCommunicationInterface.h
        inc/drivers/interfaces/RgbLedInterface.h
)

set_target_properties(drivers_interfaces PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(drivers_interfaces PUBLIC inc)
