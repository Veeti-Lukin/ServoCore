add_library(drivers_interfaces
        inc/drivers/interfaces/BufferedSerialCommunicationInterface.h
        inc/drivers/interfaces/RgbLedInterface.h
        inc/drivers/interfaces/TimerInterface.h
        inc/drivers/interfaces/ClockInterface.h
)

set_target_properties(drivers_interfaces PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(drivers_interfaces PUBLIC inc)
